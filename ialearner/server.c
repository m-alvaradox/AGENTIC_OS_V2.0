#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "server.h"
#include "client_handler.h"

int iniciarServidor(int puerto)
{
    int server_fd;

    struct sockaddr_in servidor;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        perror("socket servidor");
        return -1;
    }

    int opcion = 1;
    if (setsockopt(server_fd,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &opcion,
                   sizeof(opcion)) == -1)
    {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

    memset(&servidor, 0, sizeof(servidor)); // limpiar campos de la estructura

    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(puerto);
    servidor.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0

    if (bind(server_fd, (struct sockaddr *)&servidor,
             sizeof(servidor)) == -1)
    {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 10) == -1)
    {
        perror("listen");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

static void manejarCliente(int client_fd, SessionContext *session)
{
    printf("[VENTANA] Conexion de datos establecida.\n");

    ClientInfo *info = malloc(sizeof(ClientInfo));

    if (info == NULL)
    {
        perror("malloc");
        close(client_fd);
        return;
    }

    info->client_fd = client_fd;
    info->documento = NULL;
    info->longitud = 0;
    info->capacidad = 0;
    info->session = session;

    pthread_t hilo;

    if (pthread_create(&hilo,
                       NULL,
                       atenderCliente,
                       info) != 0)
    {
        perror("pthread_create");

        close(client_fd);

        free(info);

        return;
    }

    if (agregarThread(&session->threadManager, hilo) == -1)
    {
        // evitar hilos huerfanos despues del fallo
        // solicitud para terminar el hilo
        pthread_cancel(hilo);
        pthread_join(hilo, NULL);
    }
}

void aceptarClientes(int server_fd, SessionContext *session)
{
    int client_fd;

    struct sockaddr_in cliente;

    socklen_t cliente_len = sizeof(cliente);
    struct pollfd pfd;

    pfd.fd = server_fd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    while (sessionEstaActiva(session))
    {
        int poll_result = poll(&pfd, 1, 200);

        if (poll_result == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            perror("poll");
            break;
        }

        if (poll_result == 0)
        {
            continue;
        }

        if ((pfd.revents & POLLIN) == 0)
        {
            continue;
        }

        client_fd = accept(
            server_fd,
            (struct sockaddr *)&cliente,
            &cliente_len);

        if (client_fd == -1)
        {
            if (!sessionEstaActiva(session) ||
                errno == EBADF ||
                errno == EINVAL ||
                errno == ENOTCONN ||
                errno == ECONNABORTED ||
                errno == EINTR)
            {
                break;
            }

            perror("accept");
            continue;
        }

        manejarCliente(client_fd, session);
    }
}

void *aceptarClientesVentana(void *arg)
{
    WindowServiceArgs *args = (WindowServiceArgs *)arg;

    if (args == NULL || args->session == NULL)
    {
        return NULL;
    }

    int server_fd = args->server_fd;
    SessionContext *session = args->session;
    struct sockaddr_in cliente;
    socklen_t cliente_len = sizeof(cliente);

    free(args);

    while (sessionEstaActiva(session))
    {
        int client_fd = accept(server_fd,
                               (struct sockaddr *)&cliente,
                               &cliente_len);

        if (client_fd == -1)
        {
            if (errno == EINTR || errno == EBADF || errno == EINVAL ||
                errno == ENOTCONN || errno == ECONNABORTED)
            {
                break;
            }

            perror("accept ventana");
            break;
        }

        manejarCliente(client_fd, session);
    }

    close(server_fd);
    return NULL;
}
