#include <unistd.h>
#include <stdio.h>

#include "session_context.h"

int inicializarSessionContext(SessionContext *session,
                              ServerContext *server,
                              int launcherSocket)
{
    if (session == NULL || server == NULL)
    {
        return -1;
    }

    session->server = server;
    session->launcherSocket = launcherSocket;
    session->window_count = 0;
    session->sessionActiva = true;

    inicializarPerfil(&session->perfil);
    inicializarThreadManager(&session->threadManager);

    for (int i = 0; i < MAX_WINDOW_SERVERS; i++)
    {
        session->window_server_fds[i] = -1;
        session->window_ports[i] = -1;
    }

    if (pthread_mutex_init(&session->perfilMutex, NULL) != 0)
    {
        perror("pthread_mutex_init");
        liberarThreadManager(&session->threadManager);
        return -1;
    }

    if (pthread_mutex_init(&session->printMutex, NULL) != 0)
    {
        perror("pthread_mutex_init");
        pthread_mutex_destroy(&session->perfilMutex);
        liberarThreadManager(&session->threadManager);
        return -1;
    }

    return 0;
}

void liberarSessionContext(SessionContext *session)
{
    if (session == NULL)
    {
        return;
    }

    for (int i = 0; i < session->window_count; i++)
    {
        if (session->window_server_fds[i] != -1)
        {
            close(session->window_server_fds[i]);
            session->window_server_fds[i] = -1;
        }
    }

    unirThreads(&session->threadManager);
    liberarThreadManager(&session->threadManager);

    if (session->launcherSocket != -1)
    {
        close(session->launcherSocket);
        session->launcherSocket = -1;
    }

    pthread_mutex_destroy(&session->printMutex);
    pthread_mutex_destroy(&session->perfilMutex);
}
