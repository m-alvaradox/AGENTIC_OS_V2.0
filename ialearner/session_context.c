#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

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
    session->tipoUsuarioActual = USER_NO_DETECTADO;
    inicializarThreadManager(&session->threadManager);
    session->detectionThreads = server->detectionThreads;
    session->detectorPool = NULL;
    session->detectionBatch = NULL;
    session->batchSize = 0;
    session->nextTask = 0;
    session->completedTasks = 0;
    session->batchSequence = 0;
    session->currentBatchId = 0;
    session->detectorStop = false;
    session->detectorPoolStarted = false;

    if (session->detectionThreads <= 0)
    {
        session->detectionThreads = DEFAULT_DETECTION_THREADS;
    }

    if (inicializarSentenceQueue(&session->sentenceQueue) == -1)
    {
        liberarThreadManager(&session->threadManager);
        return -1;
    }

    for (int i = 0; i < MAX_WINDOW_SERVERS; i++)
    {
        session->window_server_fds[i] = -1;
        session->window_ports[i] = -1;
    }

    if (pthread_mutex_init(&session->perfilMutex, NULL) != 0)
    {
        perror("pthread_mutex_init");
        liberarSentenceQueue(&session->sentenceQueue);
        liberarThreadManager(&session->threadManager);
        return -1;
    }

    if (pthread_mutex_init(&session->printMutex, NULL) != 0)
    {
        perror("pthread_mutex_init");
        pthread_mutex_destroy(&session->perfilMutex);
        liberarSentenceQueue(&session->sentenceQueue);
        liberarThreadManager(&session->threadManager);
        return -1;
    }

    if (pthread_mutex_init(&session->sessionMutex, NULL) != 0)
    {
        perror("pthread_mutex_init");
        pthread_mutex_destroy(&session->printMutex);
        pthread_mutex_destroy(&session->perfilMutex);
        liberarSentenceQueue(&session->sentenceQueue);
        liberarThreadManager(&session->threadManager);
        return -1;
    }

    if (pthread_mutex_init(&session->processingMutex, NULL) != 0)
    {
        perror("pthread_mutex_init");
        pthread_mutex_destroy(&session->sessionMutex);
        pthread_mutex_destroy(&session->printMutex);
        pthread_mutex_destroy(&session->perfilMutex);
        liberarSentenceQueue(&session->sentenceQueue);
        liberarThreadManager(&session->threadManager);
        return -1;
    }

    if (pthread_mutex_init(&session->detectorMutex, NULL) != 0)
    {
        perror("pthread_mutex_init");
        pthread_mutex_destroy(&session->processingMutex);
        pthread_mutex_destroy(&session->sessionMutex);
        pthread_mutex_destroy(&session->printMutex);
        pthread_mutex_destroy(&session->perfilMutex);
        liberarSentenceQueue(&session->sentenceQueue);
        liberarThreadManager(&session->threadManager);
        return -1;
    }

    if (pthread_cond_init(&session->detectorCond, NULL) != 0)
    {
        perror("pthread_cond_init");
        pthread_mutex_destroy(&session->detectorMutex);
        pthread_mutex_destroy(&session->processingMutex);
        pthread_mutex_destroy(&session->sessionMutex);
        pthread_mutex_destroy(&session->printMutex);
        pthread_mutex_destroy(&session->perfilMutex);
        liberarSentenceQueue(&session->sentenceQueue);
        liberarThreadManager(&session->threadManager);
        return -1;
    }

    if (pthread_cond_init(&session->batchCompleteCond, NULL) != 0)
    {
        perror("pthread_cond_init");
        pthread_cond_destroy(&session->detectorCond);
        pthread_mutex_destroy(&session->detectorMutex);
        pthread_mutex_destroy(&session->processingMutex);
        pthread_mutex_destroy(&session->sessionMutex);
        pthread_mutex_destroy(&session->printMutex);
        pthread_mutex_destroy(&session->perfilMutex);
        liberarSentenceQueue(&session->sentenceQueue);
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

    pthread_mutex_lock(&session->sessionMutex);

    for (int i = 0; i < session->window_count; i++)
    {
        if (session->window_server_fds[i] != -1)
        {
            close(session->window_server_fds[i]);
            session->window_server_fds[i] = -1;
        }
    }

    pthread_mutex_unlock(&session->sessionMutex);

    unirThreads(&session->threadManager);
    liberarThreadManager(&session->threadManager);
    liberarSentenceQueue(&session->sentenceQueue);
    free(session->detectorPool);
    free(session->detectionBatch);

    if (session->launcherSocket != -1)
    {
        close(session->launcherSocket);
        session->launcherSocket = -1;
    }

    pthread_mutex_destroy(&session->sessionMutex);
    pthread_mutex_destroy(&session->processingMutex);
    pthread_cond_destroy(&session->batchCompleteCond);
    pthread_cond_destroy(&session->detectorCond);
    pthread_mutex_destroy(&session->detectorMutex);
    pthread_mutex_destroy(&session->printMutex);
    pthread_mutex_destroy(&session->perfilMutex);
}

bool sessionEstaActiva(SessionContext *session)
{
    bool activa;

    if (session == NULL)
    {
        return false;
    }

    pthread_mutex_lock(&session->sessionMutex);
    activa = session->sessionActiva;
    pthread_mutex_unlock(&session->sessionMutex);

    return activa;
}

void establecerSessionActiva(SessionContext *session,
                             bool activa)
{
    if (session == NULL)
    {
        return;
    }

    pthread_mutex_lock(&session->sessionMutex);
    session->sessionActiva = activa;
    pthread_mutex_unlock(&session->sessionMutex);

    if (!activa)
    {
        despertarSentenceQueue(&session->sentenceQueue);
    }
}
