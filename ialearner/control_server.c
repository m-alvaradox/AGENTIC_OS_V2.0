#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>

#include "control_server.h"
#include "config.h"
#include "protocol.h"

void *ejecutarControlServer(void *arg)
{
    ServerContext *context = (ServerContext *)arg;

    int serverFD;
    int launcherFD;

    struct sockaddr_in serverAddr;
    struct sockaddr_in launcherAddr;

    socklen_t launcherLen = sizeof(launcherAddr);

    serverFD = socket(AF_INET, SOCK_STREAM, 0);

    if (serverFD == -1)
    {
        perror("socket");

        return NULL;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(CONTROL_SERVICE_PORT);

    if (bind(serverFD,
             (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) == -1)
    {
        perror("bind");

        close(serverFD);

        return NULL;
    }

    if (listen(serverFD, 1) == -1)
    {
        perror("listen");

        close(serverFD);

        return NULL;
    }

    launcherFD = accept(serverFD,
                        (struct sockaddr *)&launcherAddr,
                        &launcherLen);

    if (launcherFD == -1)
    {
        perror("accept");

        close(serverFD);

        return NULL;
    }

    context->launcherSocket = launcherFD;

    printf("Launcher conectado.\n");

    ControlCommand comando;
    int seguir = 1;
    ssize_t r;

    while (seguir)
    {
        r = recv(launcherFD,
                 &comando,
                 sizeof(ControlCommand),
                 0);

        if (r > 0)
        {
            switch (comando)
            {
            case CMD_START:
                printf("Sesion iniciada.\n");
                break;

            case CMD_END:
                printf("Sesion finalizada.\n");

                context->sessionActiva = false;

                if (context->server_fd != -1)
                {
                    close(context->server_fd);
                    context->server_fd = -1;
                }

                seguir = 0;

                break;

            default:
                printf("Comando invalido.\n");
                break;
            }

            continue;
        }

        if (r == 0)
        {
            // launcher closed connection
            printf("Launcher cerró la conexión.\n");
            break;
        }

        // r == -1
        if (errno == EINTR)
        {
            continue; // retry
        }

        perror("recv");
        break;
    }

    if (serverFD != -1)
    {
        close(serverFD);
    }

    return NULL;
}