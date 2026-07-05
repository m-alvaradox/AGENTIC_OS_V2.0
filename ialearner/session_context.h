#ifndef SESSION_CONTEXT_H
#define SESSION_CONTEXT_H

#include <stdbool.h>
#include <pthread.h>

#include "config.h"
#include "server_context.h"
#include "thread_manager.h"
#include "user_profile.h"

typedef struct
{
    ServerContext *server;

    UserProfile perfil;
    pthread_mutex_t perfilMutex;
    ThreadManager threadManager;

    int launcherSocket;
    int window_server_fds[MAX_WINDOW_SERVERS];
    int window_ports[MAX_WINDOW_SERVERS];
    int window_count;

    bool sessionActiva;

} SessionContext;

int inicializarSessionContext(SessionContext *session,
                              ServerContext *server,
                              int launcherSocket);

void liberarSessionContext(SessionContext *session);

#endif
