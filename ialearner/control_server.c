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
#include "server.h"
#include "user_profile.h"

static void cerrarServidoresVentana(ServerContext *context)
{
    if (context == NULL)
    {
        return;
    }

    for (int i = 0; i < context->window_count; i++)
    {
        if (context->window_server_fds[i] != -1)
        {
            close(context->window_server_fds[i]);
            context->window_server_fds[i] = -1;
        }

        context->window_ports[i] = -1;
    }

    context->window_count = 0;
}

static void enviarPerfilUsuario(ServerContext *context)
{
    if (context == NULL || context->launcherSocket == -1)
    {
        return;
    }

    UserContext contextoUsuario;

    contextoUsuario.tipo =
        clasificarUsuario(&context->perfil);

    if (send(context->launcherSocket,
             &contextoUsuario,
             sizeof(UserContext),
             0) == -1)
    {
        perror("send");
    }
}

void *ejecutarControlServer(void *arg)
{
    ServerContext *context = (ServerContext *)arg;

    int serverFD;

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

    while (1)
    {
        launcherLen = sizeof(launcherAddr);

        int launcherFD = accept(serverFD,
                                (struct sockaddr *)&launcherAddr,
                                &launcherLen);

        if (launcherFD == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }

            perror("accept");
            break;
        }

        context->launcherSocket = launcherFD;
        context->sessionActiva = true;

        printf("Launcher conectado.\n");

        ControlMessage mensaje;
        int seguir = 1;
        ssize_t r;

        while (seguir)
        {
            r = recv(launcherFD,
                     &mensaje,
                     sizeof(ControlMessage),
                     0);

            if (r > 0)
            {
                switch (mensaje.comando)
                {
                case CMD_START:
                    printf("Sesion iniciada.\n");
                    break;

                case CMD_OPEN_WINDOW:
                    printf("Abrir ventana en puerto %d.\n", mensaje.puerto);

                    if (mensaje.puerto <= 0)
                    {
                        break;
                    }

                    if (context->window_count >= MAX_WINDOW_SERVERS)
                    {
                        fprintf(stderr, "No hay espacio para mas ventanas.\n");
                        break;
                    }

                    {
                        int server_fd = iniciarServidor(mensaje.puerto);
                        if (server_fd == -1)
                        {
                            break;
                        }

                        context->window_server_fds[context->window_count] = server_fd;
                        context->window_ports[context->window_count] = mensaje.puerto;
                        context->window_count++;

                        WindowServiceArgs *args = malloc(sizeof(*args));
                        if (args == NULL)
                        {
                            perror("malloc");
                            close(server_fd);
                            break;
                        }

                        args->server_fd = server_fd;
                        args->puerto = mensaje.puerto;
                        args->contexto = context;

                        pthread_t hilo;
                        if (pthread_create(&hilo,
                                           NULL,
                                           aceptarClientesVentana,
                                           args) != 0)
                        {
                            perror("pthread_create");
                            close(server_fd);
                            free(args);
                            break;
                        }

                        if (agregarThread(&context->threadManager, hilo) == -1)
                        {
                            pthread_cancel(hilo);
                            pthread_join(hilo, NULL);
                            close(server_fd);
                            free(args);
                            break;
                        }
                    }
                    break;

                case CMD_CLOSE_WINDOW:
                    printf("Cerrar ventana en puerto %d.\n", mensaje.puerto);

                    for (int i = 0; i < context->window_count; i++)
                    {
                        if (context->window_ports[i] == mensaje.puerto &&
                            context->window_server_fds[i] != -1)
                        {
                            close(context->window_server_fds[i]);
                            context->window_server_fds[i] = -1;
                            break;
                        }
                    }
                    break;

                case CMD_END:
                    printf("Sesion finalizada.\n");

                    cerrarServidoresVentana(context);
                    enviarPerfilUsuario(context);
                    inicializarPerfil(&context->perfil);

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
                printf("Launcher cerró la conexión.\n");
                cerrarServidoresVentana(context);
                break;
            }

            if (errno == EINTR)
            {
                continue;
            }

            perror("recv");
            cerrarServidoresVentana(context);
            break;
        }

        close(launcherFD);
        context->launcherSocket = -1;
    }

    if (serverFD != -1)
    {
        close(serverFD);
    }

    return NULL;
}
