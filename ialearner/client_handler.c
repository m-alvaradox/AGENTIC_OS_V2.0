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

        pthread_mutex_lock(&session->printMutex);
        printf("[Loader] Procesando lote de %d oracion(es) con P=%d.\n",
               extraidas,
               p);
        pthread_mutex_unlock(&session->printMutex);

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
            UserType tipoActual;
            DocumentClass clase;

            if (hiloCreado[i])
            {
                pthread_join(hilos[i], NULL);
                hiloCreado[i] = false;
            }

            clase = tasks[i].resultado.clase;

            pthread_mutex_lock(&session->perfilMutex);
            registrarDocumento(&session->perfil,
                               clase);
            session->tipoUsuarioActual =
                determinarTipoUsuario(&session->perfil);
            tipoActual = session->tipoUsuarioActual;
            pthread_mutex_unlock(&session->perfilMutex);

            pthread_mutex_lock(&session->printMutex);
            printf("[Detector] Oracion clasificada como: %s.\n",
                   nombreClaseDocumento(clase));
            printf("[Perfil] Tipo de usuario actual: %s.\n",
                   nombreTipoUsuario(tipoActual));
            pthread_mutex_unlock(&session->printMutex);

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

    pthread_mutex_lock(&session->printMutex);
    printf("[Loader] Activo. Esperando lotes de %d oracion(es).\n", p);
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

        pthread_mutex_lock(&session->printMutex);
        printf("[Loader] Lote completo. Activando detectores.\n");
        pthread_mutex_unlock(&session->printMutex);

        procesarColaPendiente(session, false);
    }

    pthread_mutex_lock(&session->printMutex);
    printf("[Loader] Finalizado.\n");
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
    printf("[Ventana] Oracion encolada. Pendientes: %d/%d.\n",
           pendientes,
           info->session->detectionThreads);
    pthread_mutex_unlock(&info->session->printMutex);

    info->longitud = 0;
}
