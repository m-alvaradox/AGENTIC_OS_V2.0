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
    int server_fd;
    pthread_t controlThread;
    ServerContext contexto;

    memset(&contexto, 0, sizeof(contexto));

    server_fd = iniciarServidor(DOCUMENT_SERVICE_PORT);

    if (server_fd == -1)
    {
        return EXIT_FAILURE;
    }

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

        close(server_fd);

        return EXIT_FAILURE;
    }

    inicializarPerfil(&contexto.perfil);

    if (pthread_mutex_init(&contexto.perfilMutex, NULL) != 0)
    {
        perror("pthread_mutex_init");

        liberarDiccionario(contexto.correo);
        liberarDiccionario(contexto.articulo);
        liberarDiccionario(contexto.reporte);

        close(server_fd);

        return EXIT_FAILURE;
    }

    inicializarThreadManager(&contexto.threadManager);

    contexto.launcherSocket = -1;
    contexto.server_fd = server_fd;
    contexto.window_count = 0;
    for (int i = 0; i < MAX_WINDOW_SERVERS; i++)
    {
        contexto.window_server_fds[i] = -1;
        contexto.window_ports[i] = -1;
    }
    contexto.sessionActiva = true;

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
        pthread_mutex_destroy(&contexto.perfilMutex);

        close(server_fd);

        return EXIT_FAILURE;
    }

    printf("=== IA Learner Data Center ===\n");
    printf("Servidor del Data Center iniciado...\n");
    printf("Esperando conexiones...\n\n");

    aceptarClientes(server_fd, &contexto);

    pthread_join(controlThread, NULL);

    esperarThreads(&contexto.threadManager);

    if (contexto.server_fd != -1)
    {
        close(contexto.server_fd);
        contexto.server_fd = -1;
    }

    liberarThreadManager(&contexto.threadManager);
    pthread_mutex_destroy(&contexto.perfilMutex);
    liberarDiccionario(contexto.correo);
    liberarDiccionario(contexto.articulo);
    liberarDiccionario(contexto.reporte);

    return EXIT_SUCCESS;
}
