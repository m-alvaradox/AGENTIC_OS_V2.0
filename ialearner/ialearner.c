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

int main(void)
{
    pthread_t controlThread;
    ServerContext contexto;

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
