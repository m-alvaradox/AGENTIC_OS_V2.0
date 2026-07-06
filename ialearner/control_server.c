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
#include "session_context.h"
#include "user_profile.h"

typedef struct
{
    ServerContext *server;
    int launcherFD;

} ControlSessionArgs;

static int asignarPuertoVentana(ServerContext *server)
{
    int puerto;

    pthread_mutex_lock(&server->puertoMutex);
    puerto = server->siguientePuertoVentana++;
    pthread_mutex_unlock(&server->puertoMutex);

    return puerto;
}

static void cerrarServidoresVentana(SessionContext *session)
{
    if (session == NULL)
    {
        return;
    }

    pthread_mutex_lock(&session->sessionMutex);

    session->sessionActiva = false;

    for (int i = 0; i < session->window_count; i++)
    {
        if (session->window_server_fds[i] != -1)
        {
            close(session->window_server_fds[i]);
            session->window_server_fds[i] = -1;
        }

        session->window_ports[i] = -1;
    }

    session->window_count = 0;

    pthread_mutex_unlock(&session->sessionMutex);
}

static void enviarRespuestaVentana(SessionContext *session,
                                   int ok,
                                   int puerto)
{
    ControlResponse respuesta;

    respuesta.ok = ok;
    respuesta.puerto = puerto;

    if (send(session->launcherSocket,
             &respuesta,
             sizeof(ControlResponse),
             0) == -1)
    {
        perror("send");
    }
}

static void enviarPerfilUsuario(SessionContext *session)
{
    if (session == NULL || session->launcherSocket == -1)
    {
        return;
    }

    UserContext contextoUsuario;

    pthread_mutex_lock(&session->perfilMutex);
    contextoUsuario.tipo =
        clasificarUsuario(&session->perfil);
    inicializarPerfil(&session->perfil);
    pthread_mutex_unlock(&session->perfilMutex);

    if (send(session->launcherSocket,
             &contextoUsuario,
             sizeof(UserContext),
             0) == -1)
    {
        perror("send");
    }
}

static void abrirVentana(SessionContext *session)
{
    pthread_mutex_lock(&session->sessionMutex);

    if (session->window_count >= MAX_WINDOW_SERVERS)
    {
        pthread_mutex_unlock(&session->sessionMutex);
        fprintf(stderr, "No hay espacio para mas ventanas.\n");
        enviarRespuestaVentana(session, 0, -1);
        return;
    }

    pthread_mutex_unlock(&session->sessionMutex);

    int puerto = asignarPuertoVentana(session->server);

    printf("Abrir ventana en puerto %d.\n", puerto);

    int server_fd = iniciarServidor(puerto);
    if (server_fd == -1)
    {
        enviarRespuestaVentana(session, 0, -1);
        return;
    }

    WindowServiceArgs *args = malloc(sizeof(*args));
    if (args == NULL)
    {
        perror("malloc");
        close(server_fd);
        enviarRespuestaVentana(session, 0, -1);
        return;
    }

    args->server_fd = server_fd;
    args->puerto = puerto;
    args->session = session;

    pthread_mutex_lock(&session->sessionMutex);

    if (session->window_count >= MAX_WINDOW_SERVERS ||
        !session->sessionActiva)
    {
        pthread_mutex_unlock(&session->sessionMutex);
        close(server_fd);
        free(args);
        enviarRespuestaVentana(session, 0, -1);
        return;
    }

    int indiceVentana = session->window_count;
    session->window_server_fds[indiceVentana] = server_fd;
    session->window_ports[indiceVentana] = puerto;
    session->window_count++;

    pthread_mutex_unlock(&session->sessionMutex);

    pthread_t hilo;
    if (pthread_create(&hilo,
                       NULL,
                       aceptarClientesVentana,
                       args) != 0)
    {
        perror("pthread_create");
        close(server_fd);
        free(args);
        pthread_mutex_lock(&session->sessionMutex);
        session->window_count--;
        session->window_server_fds[indiceVentana] = -1;
        session->window_ports[indiceVentana] = -1;
        pthread_mutex_unlock(&session->sessionMutex);
        enviarRespuestaVentana(session, 0, -1);
        return;
    }

    if (agregarThread(&session->threadManager, hilo) == -1)
    {
        pthread_cancel(hilo);
        pthread_join(hilo, NULL);
        close(server_fd);
        pthread_mutex_lock(&session->sessionMutex);
        session->window_count--;
        session->window_server_fds[indiceVentana] = -1;
        session->window_ports[indiceVentana] = -1;
        pthread_mutex_unlock(&session->sessionMutex);
        enviarRespuestaVentana(session, 0, -1);
        return;
    }

    enviarRespuestaVentana(session, 1, puerto);
}

static void cerrarVentana(SessionContext *session, int puerto)
{
    printf("Cerrar ventana en puerto %d.\n", puerto);

    pthread_mutex_lock(&session->sessionMutex);

    for (int i = 0; i < session->window_count; i++)
    {
        if (session->window_ports[i] == puerto &&
            session->window_server_fds[i] != -1)
        {
            close(session->window_server_fds[i]);
            session->window_server_fds[i] = -1;
            break;
        }
    }

    pthread_mutex_unlock(&session->sessionMutex);
}

static void *atenderLauncher(void *arg)
{
    ControlSessionArgs *args = (ControlSessionArgs *)arg;

    if (args == NULL || args->server == NULL)
    {
        free(args);
        return NULL;
    }

    SessionContext session;

    if (inicializarSessionContext(&session,
                                  args->server,
                                  args->launcherFD) == -1)
    {
        close(args->launcherFD);
        free(args);
        return NULL;
    }

    free(args);

    printf("Launcher conectado.\n");

    ControlMessage mensaje;
    int seguir = 1;
    ssize_t r;

    while (seguir)
    {
        r = recv(session.launcherSocket,
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
                abrirVentana(&session);
                break;

            case CMD_CLOSE_WINDOW:
                cerrarVentana(&session, mensaje.puerto);
                break;

            case CMD_END:
                printf("Sesion finalizada.\n");

                cerrarServidoresVentana(&session);
                enviarPerfilUsuario(&session);

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
            cerrarServidoresVentana(&session);
            break;
        }

        if (errno == EINTR)
        {
            continue;
        }

        perror("recv");
        cerrarServidoresVentana(&session);
        break;
    }

    liberarSessionContext(&session);

    return NULL;
}

void *ejecutarControlServer(void *arg)
{
    ServerContext *server = (ServerContext *)arg;

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

    int opcion = 1;
    if (setsockopt(serverFD,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   &opcion,
                   sizeof(opcion)) == -1)
    {
        perror("setsockopt");
        close(serverFD);
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

    if (listen(serverFD, MAX_SESSIONS) == -1)
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

        ControlSessionArgs *args = malloc(sizeof(*args));
        if (args == NULL)
        {
            perror("malloc");
            close(launcherFD);
            continue;
        }

        args->server = server;
        args->launcherFD = launcherFD;

        pthread_t hilo;
        if (pthread_create(&hilo, NULL, atenderLauncher, args) != 0)
        {
            perror("pthread_create");
            close(launcherFD);
            free(args);
            continue;
        }

        if (agregarThread(&server->threadManager, hilo) == -1)
        {
            pthread_cancel(hilo);
            pthread_join(hilo, NULL);
            close(launcherFD);
            continue;
        }
    }

    if (serverFD != -1)
    {
        close(serverFD);
    }

    return NULL;
}
