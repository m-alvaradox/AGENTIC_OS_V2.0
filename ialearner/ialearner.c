#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "server_context.h"
#include "dictionary.h"
#include "user_profile.h"
#include "config.h"

int main(void)
{
    int server_fd;

    server_fd = iniciarServidor(DOCUMENT_SERVICE_PORT);

    if (server_fd == -1)
    {
        return EXIT_FAILURE;
    }

    // Definir contexto

    ServerContext contexto = {
        .correo = cargarDiccionario(DICTIONARY_DIRECTORY EMAIL_DICTIONARY_FILE, EMAIL_CLASS_NAME),
        .articulo = cargarDiccionario(DICTIONARY_DIRECTORY ARTICLE_DICTIONARY_FILE, ARTICLE_CLASS_NAME),
        .reporte = cargarDiccionario(DICTIONARY_DIRECTORY REPORT_DICTIONARY_FILE, REPORT_CLASS_NAME),
    };

    if (contexto.correo == NULL ||
        contexto.articulo == NULL ||
        contexto.reporte == NULL)
    {
        fprintf(stderr, "Error al cargar los diccionarios\n");

        liberarDiccionario(contexto.correo);
        liberarDiccionario(contexto.articulo);
        liberarDiccionario(contexto.reporte);
    }

    inicializarPerfil(&contexto.perfil);

    inicializarThreadManager(&contexto.threadManager);

    contexto.launcherSocket = -1;

    contexto.sessionActiva = true;

    pthread_t controlThread;

    if (pthread_create(&controlThread,
                       NULL,
                       ejecutarControlServer,
                       &contexto) != 0)
    {
        perror("pthread_create");

        liberarDiccionario(contexto.correo);
        liberarDiccionario(contexto.articulo);
        liberarDiccionario(contexto.reporte);

        return EXIT_FAILURE;
    }

    printf("=== IA Learner Data Center ===\n");
    printf("Servidor del Data Center iniciado...\n");
    printf("Esperando conexiones...\n\n");

    aceptarClientes(server_fd, &contexto);

    close(server_fd);

    pthread_join(controlThread, NULL);

    esperarThreads(&contexto.threadManager);

    UserContext contextoUsuario;

    contextoUsuario.tipo =
        clasificarUsuario(&contexto.perfil);

    // send to laucher
        send(contexto.launcherSocket,
     &contextoUsuario,
     sizeof(UserContext),
     0);

    liberarDiccionario(contexto.correo);
    liberarDiccionario(contexto.articulo);
    liberarDiccionario(contexto.reporte);

    return EXIT_SUCCESS;
}