#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>

#include "client_handler.h"
#include "classifier.h"
#include "config.h"

static const char *nombreClaseDocumento(DocumentClass clase)
{
    switch (clase)
    {
    case DOC_CORREO:
        return "Correo electronico";

    case DOC_ARTICULO:
        return "Articulo cientifico";

    case DOC_REPORTE:
        return "Reporte";

    default:
        return "Sin clasificar";
    }
}

static const char *nombreTipoUsuario(UserType tipo)
{
    switch (tipo)
    {
    case USER_ADMINISTRATIVO:
        return "Personal administrativo";

    case USER_TECNICO:
        return "Personal tecnico";

    case USER_PROFESOR:
        return "Profesor";

    case USER_ESTUDIANTE:
        return "Estudiante";

    default:
        return "No detectado";
    }
}

static void *ejecutarDetector(void *arg)
{
    SessionContext *session = (SessionContext *)arg;

    for (;;)
    {
        char *oracion;
        ClassificationResult resultado;
        UserType tipoActual;
        int numeroTarea;
        int totalTareas;
        int loteId;

        pthread_mutex_lock(&session->detectorMutex);

        while (!session->detectorStop &&
               session->nextTask >= session->batchSize)
        {
            pthread_cond_wait(&session->detectorCond,
                              &session->detectorMutex);
        }

        if (session->detectorStop)
        {
            pthread_mutex_unlock(&session->detectorMutex);
            break;
        }

        numeroTarea = session->nextTask++;
        totalTareas = session->batchSize;
        loteId = session->currentBatchId;
        oracion = session->detectionBatch[numeroTarea];
        pthread_mutex_unlock(&session->detectorMutex);

        resultado = clasificarDocumento(oracion,
                                        session->server->correo,
                                        session->server->articulo,
                                        session->server->reporte);

        /* La decision se actualiza apenas este detector termina. */
        pthread_mutex_lock(&session->perfilMutex);
        registrarDocumento(&session->perfil, resultado.clase);
        session->tipoUsuarioActual = determinarTipoUsuario(&session->perfil);
        tipoActual = session->tipoUsuarioActual;
        pthread_mutex_unlock(&session->perfilMutex);

        pthread_mutex_lock(&session->printMutex);
#if SHOW_DOCUMENT_DEBUG
        printf("[LOTE %02d][%d/%d] Texto: \"%.*s\"\n",
               loteId,
               numeroTarea + 1,
               totalTareas,
               (int)strcspn(oracion, "\r\n"),
               oracion);
#endif
        printf("[LOTE %02d][%d/%d] Clase: %-20s | "
               "coincidencias C:%d A:%d R:%d | usuario: %s\n",
               loteId,
               numeroTarea + 1,
               totalTareas,
               nombreClaseDocumento(resultado.clase),
               resultado.coincidenciasCorreo,
               resultado.coincidenciasArticulo,
               resultado.coincidenciasReporte,
               nombreTipoUsuario(tipoActual));
        pthread_mutex_unlock(&session->printMutex);

        free(oracion);

        pthread_mutex_lock(&session->detectorMutex);
        session->completedTasks++;
        if (session->completedTasks == session->batchSize)
        {
            pthread_cond_signal(&session->batchCompleteCond);
        }
        pthread_mutex_unlock(&session->detectorMutex);
    }

    return NULL;
}

int iniciarPoolDetectores(SessionContext *session)
{
    int creados = 0;

    if (session == NULL || session->detectionThreads <= 0)
    {
        return -1;
    }

    session->detectorPool = calloc((size_t)session->detectionThreads,
                                   sizeof(pthread_t));
    session->detectionBatch = calloc((size_t)session->detectionThreads,
                                     sizeof(char *));

    if (session->detectorPool == NULL || session->detectionBatch == NULL)
    {
        free(session->detectorPool);
        free(session->detectionBatch);
        session->detectorPool = NULL;
        session->detectionBatch = NULL;
        return -1;
    }

    for (int i = 0; i < session->detectionThreads; i++)
    {
        if (pthread_create(&session->detectorPool[i], NULL,
                           ejecutarDetector, session) != 0)
        {
            perror("pthread_create detector");
            pthread_mutex_lock(&session->detectorMutex);
            session->detectorStop = true;
            pthread_cond_broadcast(&session->detectorCond);
            pthread_mutex_unlock(&session->detectorMutex);

            for (int j = 0; j < creados; j++)
            {
                pthread_join(session->detectorPool[j], NULL);
            }

            free(session->detectorPool);
            free(session->detectionBatch);
            session->detectorPool = NULL;
            session->detectionBatch = NULL;
            return -1;
        }
        creados++;
    }

    session->detectorPoolStarted = true;
    return 0;
}

void detenerPoolDetectores(SessionContext *session)
{
    if (session == NULL || !session->detectorPoolStarted)
    {
        return;
    }

    pthread_mutex_lock(&session->detectorMutex);
    session->detectorStop = true;
    pthread_cond_broadcast(&session->detectorCond);
    pthread_mutex_unlock(&session->detectorMutex);

    for (int i = 0; i < session->detectionThreads; i++)
    {
        pthread_join(session->detectorPool[i], NULL);
    }

    session->detectorPoolStarted = false;
}

void procesarColaPendiente(SessionContext *session,
                            bool forzar)
{
    int p;
    int extraidas;

    if (session == NULL)
    {
        return;
    }

    pthread_mutex_lock(&session->processingMutex);

    p = session->detectionThreads;
    if (p <= 0)
    {
        p = 1;
    }

    if (!forzar &&
        cantidadOraciones(&session->sentenceQueue) < p)
    {
        pthread_mutex_unlock(&session->processingMutex);
        return;
    }

    while (forzar ||
           cantidadOraciones(&session->sentenceQueue) >= p)
    {
        extraidas = extraerOraciones(&session->sentenceQueue,
                                     session->detectionBatch,
                                     p);

        if (extraidas == 0)
        {
            break;
        }

        pthread_mutex_lock(&session->printMutex);
        printf("\n[LOADER] Preparando lote | oraciones=%d | limite P=%d\n",
               extraidas, p);
        pthread_mutex_unlock(&session->printMutex);

        pthread_mutex_lock(&session->detectorMutex);
        session->batchSize = extraidas;
        session->nextTask = 0;
        session->completedTasks = 0;
        session->currentBatchId = ++session->batchSequence;
        printf("[LOADER] Lote %02d iniciado: despertando %d detector(es).\n",
               session->currentBatchId, extraidas);
        pthread_cond_broadcast(&session->detectorCond);

        while (session->completedTasks < session->batchSize)
        {
            pthread_cond_wait(&session->batchCompleteCond,
                              &session->detectorMutex);
        }

        session->batchSize = 0;
        printf("[LOADER] Lote %02d completado.\n",
               session->currentBatchId);
        pthread_mutex_unlock(&session->detectorMutex);

        if (!forzar)
        {
            break;
        }
    }

    pthread_mutex_unlock(&session->processingMutex);
}

void *ejecutarLoader(void *arg)
{
    SessionContext *session = (SessionContext *)arg;
    int p;

    if (session == NULL)
    {
        return NULL;
    }

    p = session->detectionThreads;
    if (p <= 0)
    {
        p = 1;
    }

    pthread_mutex_lock(&session->printMutex);
    printf("[LOADER] Activo | tamano de lote P=%d | detectores suspendidos.\n", p);
    pthread_mutex_unlock(&session->printMutex);

    while (sessionEstaActiva(session))
    {
        pthread_mutex_lock(&session->sentenceQueue.mutex);

        while (session->sentenceQueue.cantidad < p &&
               sessionEstaActiva(session))
        {
            pthread_cond_wait(&session->sentenceQueue.cond,
                              &session->sentenceQueue.mutex);
        }

        pthread_mutex_unlock(&session->sentenceQueue.mutex);

        if (!sessionEstaActiva(session))
        {
            break;
        }

        procesarColaPendiente(session, false);
    }

    pthread_mutex_lock(&session->printMutex);
    printf("[LOADER] Finalizado.\n");
    pthread_mutex_unlock(&session->printMutex);

    return NULL;
}

void *atenderCliente(void *arg)
{
    ClientInfo *info = (ClientInfo *)arg;

    char letra;
    ssize_t r;

    for (;;)
    {
        if (!sessionEstaActiva(info->session))
        {
            break;
        }

        r = recv(info->client_fd,
                 &letra,
                 sizeof(char),
                 0);

        if (r > 0)
        {
            if (agregarCaracter(info, letra) == -1)
            {
                liberarCliente(info);
                return NULL;
            }

            if (letra == '\n')
            {
                procesarDocumento(info);
            }

            continue;
        }

        if (r == 0)
        {
            // peer closed connection normally
            break;
        }

        // r == -1
        if (errno == EINTR)
        {
            continue; // retry
        }

        perror("recv");
        liberarCliente(info);
        return NULL;
    }

    if (info->longitud > 0)
    {
        procesarDocumento(info);
    }

    liberarCliente(info);

    return NULL;
}

void liberarCliente(ClientInfo *info)
{
    if (info == NULL)
    {
        return;
    }

    free(info->documento);

    close(info->client_fd);

    free(info);
}

int agregarCaracter(ClientInfo *info, char letra)
{
    if (info == NULL)
    {
        return -1;
    }

    if (info->longitud == info->capacidad)
    {
        size_t nuevaCapacidad;

        if (info->capacidad == 0)
        {
            nuevaCapacidad = 256;
        }
        else
        {
            nuevaCapacidad = info->capacidad * 2;
        }

        // secure realloc
        char *temp = realloc(info->documento, nuevaCapacidad);

        if (temp == NULL)
        {
            perror("realloc");
            return -1;
        }

        info->documento = temp;
        info->capacidad = nuevaCapacidad;
    }

    // Save word
    info->documento[info->longitud] = letra;
    info->longitud++;
    return 0;
}

void procesarDocumento(ClientInfo *info)
{
    int pendientes;

    if (info == NULL)
    {
        return;
    }

    if (info->longitud == 0)
    {
        return;
    }

    if (info->longitud == info->capacidad)
    { // Para asegurar espacio para el carácter nulo \0... 1 byte extra
        char *temp = realloc(info->documento,
                             info->capacidad + 1);

        if (temp == NULL)
        {
            perror("realloc");
            return;
        }

        info->documento = temp;
        info->capacidad += 1;
    }

    info->documento[info->longitud] = '\0'; // Null-terminate the string

    if (encolarOracion(&info->session->sentenceQueue,
                       info->documento) == -1)
    {
        perror("encolarOracion");
        return;
    }

    pendientes = cantidadOraciones(&info->session->sentenceQueue);

    pthread_mutex_lock(&info->session->printMutex);
    printf("[COLA] Oracion recibida | pendientes=%d | necesarias=%d\n",
           pendientes,
           info->session->detectionThreads);
    pthread_mutex_unlock(&info->session->printMutex);

    info->longitud = 0;
}
