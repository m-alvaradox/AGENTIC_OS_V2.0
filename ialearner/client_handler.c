#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

#include "client_handler.h"
#include "classifier.h"
#include "config.h"

typedef struct
{
    SessionContext *session;
    char *oracion;
    ClassificationResult resultado;
} DetectionTask;

static void *detectarOracion(void *arg)
{
    DetectionTask *task = (DetectionTask *)arg;
    ClassificationResult resultado;

    if (task == NULL ||
        task->session == NULL ||
        task->oracion == NULL)
    {
        return NULL;
    }

    resultado = clasificarDocumento(task->oracion,
                                    task->session->server->correo,
                                    task->session->server->articulo,
                                    task->session->server->reporte);

    task->resultado = resultado;

#if SHOW_DOCUMENT_DEBUG
    pthread_mutex_lock(&task->session->printMutex);
    printf("\n========== ORACION RECIBIDA ==========\n");
    printf("%s\n", task->oracion);
    imprimirClasificacion(&resultado);
    pthread_mutex_unlock(&task->session->printMutex);
#endif

    return NULL;
}

void procesarColaPendiente(SessionContext *session,
                            bool forzar)
{
    int p;
    char **oraciones;
    DetectionTask *tasks;
    pthread_t *hilos;
    bool *hiloCreado;
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

    oraciones = calloc((size_t)p, sizeof(char *));
    tasks = calloc((size_t)p, sizeof(DetectionTask));
    hilos = calloc((size_t)p, sizeof(pthread_t));
    hiloCreado = calloc((size_t)p, sizeof(bool));

    if (oraciones == NULL ||
        tasks == NULL ||
        hilos == NULL ||
        hiloCreado == NULL)
    {
        perror("calloc");
        free(oraciones);
        free(tasks);
        free(hilos);
        free(hiloCreado);
        pthread_mutex_unlock(&session->processingMutex);
        return;
    }

    while (forzar ||
           cantidadOraciones(&session->sentenceQueue) >= p)
    {
        extraidas = extraerOraciones(&session->sentenceQueue,
                                     oraciones,
                                     p);

        if (extraidas == 0)
        {
            break;
        }

        for (int i = 0; i < extraidas; i++)
        {
            tasks[i].session = session;
            tasks[i].oracion = oraciones[i];
            tasks[i].resultado.clase = DOC_SIN_CLASIFICAR;
            tasks[i].resultado.coincidenciasCorreo = 0;
            tasks[i].resultado.coincidenciasArticulo = 0;
            tasks[i].resultado.coincidenciasReporte = 0;

            if (pthread_create(&hilos[i],
                               NULL,
                               detectarOracion,
                               &tasks[i]) == 0)
            {
                hiloCreado[i] = true;
            }
            else
            {
                perror("pthread_create");
                detectarOracion(&tasks[i]);
                hiloCreado[i] = false;
            }
        }

        for (int i = 0; i < extraidas; i++)
        {
            if (hiloCreado[i])
            {
                pthread_join(hilos[i], NULL);
                hiloCreado[i] = false;
            }

            pthread_mutex_lock(&session->perfilMutex);
            registrarDocumento(&session->perfil,
                               tasks[i].resultado.clase);
            session->tipoUsuarioActual =
                determinarTipoUsuario(&session->perfil);
            pthread_mutex_unlock(&session->perfilMutex);

            free(tasks[i].oracion);
            tasks[i].oracion = NULL;
            oraciones[i] = NULL;
        }

        if (!forzar)
        {
            break;
        }
    }

    free(hiloCreado);
    free(hilos);
    free(tasks);
    free(oraciones);
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

            printf("Recibido: %c\n", letra);

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

    info->longitud = 0;
}
