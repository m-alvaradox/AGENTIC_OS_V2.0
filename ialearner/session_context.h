#ifndef SESSION_CONTEXT_H
#define SESSION_CONTEXT_H

#include <stdbool.h>
#include <pthread.h>

#include "config.h"
#include "server_context.h"
#include "sentence_queue.h"
#include "thread_manager.h"
#include "user_profile.h"

typedef struct
{
    ServerContext *server;

    UserProfile perfil;
    UserType tipoUsuarioActual;
    pthread_mutex_t perfilMutex;
    pthread_mutex_t printMutex;
    pthread_mutex_t sessionMutex;
    pthread_mutex_t processingMutex;
    ThreadManager threadManager;
    SentenceQueue sentenceQueue;
    int detectionThreads;

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

bool sessionEstaActiva(SessionContext *session);

void establecerSessionActiva(SessionContext *session,
                             bool activa);

#endif
