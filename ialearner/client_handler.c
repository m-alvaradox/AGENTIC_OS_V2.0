#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <errno.h>

#include "client_handler.h"
#include "classifier.h"
#include "config.h"

static void clasificarOracion(SessionContext *session,
                              const char *oracion)
{
    ClassificationResult resultado;

    if (session == NULL || oracion == NULL)
    {
        return;
    }

    resultado = clasificarDocumento(oracion,
                                    session->server->correo,
                                    session->server->articulo,
                                    session->server->reporte);

#if SHOW_DOCUMENT_DEBUG
    pthread_mutex_lock(&session->printMutex);
    printf("\n========== ORACION RECIBIDA ==========\n");
    printf("%s\n", oracion);
    imprimirClasificacion(&resultado);
    pthread_mutex_unlock(&session->printMutex);
#endif

    pthread_mutex_lock(&session->perfilMutex);
    registrarDocumento(&session->perfil, resultado.clase);
    pthread_mutex_unlock(&session->perfilMutex);
}

void procesarColaPendiente(SessionContext *session,
                            bool forzar)
{
    int p;
    char **oraciones;
    int extraidas;

    if (session == NULL)
    {
        return;
    }

    p = session->detectionThreads;
    if (p <= 0)
    {
        p = 1;
    }

    if (!forzar &&
        cantidadOraciones(&session->sentenceQueue) < p)
    {
        return;
    }

    oraciones = calloc((size_t)p, sizeof(char *));
    if (oraciones == NULL)
    {
        perror("calloc");
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
            clasificarOracion(session, oraciones[i]);
            free(oraciones[i]);
            oraciones[i] = NULL;
        }

        if (!forzar)
        {
            break;
        }
    }

    free(oraciones);
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
    procesarColaPendiente(info->session, false);
}
