#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "server.h"
#include "server_context.h"
#include "dictionary.h"
#include "user_profile.h"
#include "config.h"
#include "control_server.h"

static int obtenerDetectionThreads(int argc, char **argv)
{
    char *fin;
    long valor;

    if (argc < 2)
    {
        return DEFAULT_DETECTION_THREADS;
    }

    valor = strtol(argv[1], &fin, 10);

    if (*argv[1] == '\0' || *fin != '\0' || valor <= 0)
    {
        fprintf(stderr,
                "Parametro P invalido. Uso: ./ialearner [P]\n");
        return -1;
    }

    if (valor > MAX_DETECTION_THREADS)
    {
        fprintf(stderr,
                "P excede el maximo permitido (%d).\n",
                MAX_DETECTION_THREADS);
        return -1;
    }

    return (int)valor;
}

static int obtenerCpuCount(void)
{
    long cpus;

    cpus = sysconf(_SC_NPROCESSORS_ONLN);

    if (cpus <= 0)
    {
        return 1;
    }

    if (cpus > MAX_DETECTION_THREADS)
    {
        return MAX_DETECTION_THREADS;
    }

    return (int)cpus;
}

int main(int argc, char **argv)
{
    pthread_t controlThread;
    ServerContext contexto;
    int detectionThreads;
    int cpuCount;

    detectionThreads = obtenerDetectionThreads(argc, argv);
    if (detectionThreads == -1)
    {
        return EXIT_FAILURE;
    }

    cpuCount = obtenerCpuCount();

    if (detectionThreads > cpuCount)
    {
        printf("P=%d excede los CPUs disponibles (%d). "
               "Se usara P=%d.\n",
               detectionThreads,
               cpuCount,
               cpuCount);
        detectionThreads = cpuCount;
    }

    memset(&contexto, 0, sizeof(contexto));

    contexto.correo = cargarDiccionario(DICTIONARY_DIRECTORY EMAIL_DICTIONARY_FILE, EMAIL_CLASS_NAME);
    contexto.articulo = cargarDiccionario(DICTIONARY_DIRECTORY ARTICLE_DICTIONARY_FILE, ARTICLE_CLASS_NAME);
    contexto.reporte = cargarDiccionario(DICTIONARY_DIRECTORY REPORT_DICTIONARY_FILE, REPORT_CLASS_NAME);

    if (contexto.correo == NULL ||
        contexto.articulo == NULL ||
        contexto.reporte == NULL)
    {
        fprintf(stderr, "Error al cargar los diccionarios\n");

        liberarDiccionario(contexto.correo);
        liberarDiccionario(contexto.articulo);
        liberarDiccionario(contexto.reporte);

        return EXIT_FAILURE;
    }

    if (pthread_mutex_init(&contexto.puertoMutex, NULL) != 0)
    {
        perror("pthread_mutex_init");

        liberarDiccionario(contexto.correo);
        liberarDiccionario(contexto.articulo);
        liberarDiccionario(contexto.reporte);

        return EXIT_FAILURE;
    }

    inicializarThreadManager(&contexto.threadManager);

    contexto.server_fd = -1;
    contexto.detectionThreads = detectionThreads;
    contexto.cpuCount = cpuCount;
    contexto.siguientePuertoVentana = WINDOW_BASE_PORT;

    if (pthread_create(&controlThread,
                       NULL,
                       ejecutarControlServer,
                       &contexto) != 0)
    {
        perror("pthread_create");

        liberarDiccionario(contexto.correo);
        liberarDiccionario(contexto.articulo);
        liberarDiccionario(contexto.reporte);
        liberarThreadManager(&contexto.threadManager);
        pthread_mutex_destroy(&contexto.puertoMutex);

        return EXIT_FAILURE;
    }

    printf("=== IA Learner Data Center ===\n");
    printf("Servidor del Data Center iniciado...\n");
    printf("CPUs disponibles: %d\n", contexto.cpuCount);
    printf("Hilos de deteccion por lote (P): %d\n",
           contexto.detectionThreads);
    printf("Esperando conexiones...\n\n");

    pthread_join(controlThread, NULL);

    esperarThreads(&contexto.threadManager);

    if (contexto.server_fd != -1)
    {
        close(contexto.server_fd);
        contexto.server_fd = -1;
    }

    liberarThreadManager(&contexto.threadManager);
    pthread_mutex_destroy(&contexto.puertoMutex);
    liberarDiccionario(contexto.correo);
    liberarDiccionario(contexto.articulo);
    liberarDiccionario(contexto.reporte);

    return EXIT_SUCCESS;
}
