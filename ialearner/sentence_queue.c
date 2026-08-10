#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "sentence_queue.h"

static char *copiarOracion(const char *oracion)
{
    char *copia;

    if (oracion == NULL)
    {
        return NULL;
    }

    copia = malloc(strlen(oracion) + 1);
    if (copia == NULL)
    {
        return NULL;
    }

    strcpy(copia, oracion);
    return copia;
}

static int aumentarCapacidad(SentenceQueue *queue)
{
    int nuevaCapacidad;
    char **nuevasOraciones;

    if (queue == NULL)
    {
        return -1;
    }

    nuevaCapacidad = queue->capacidad * 2;
    nuevasOraciones = malloc(sizeof(char *) * nuevaCapacidad);

    if (nuevasOraciones == NULL)
    {
        return -1;
    }

    for (int i = 0; i < queue->cantidad; i++)
    {
        int indice = (queue->frente + i) % queue->capacidad;
        nuevasOraciones[i] = queue->oraciones[indice];
    }

    free(queue->oraciones);
    queue->oraciones = nuevasOraciones;
    queue->capacidad = nuevaCapacidad;
    queue->frente = 0;

    return 0;
}

int inicializarSentenceQueue(SentenceQueue *queue)
{
    if (queue == NULL)
    {
        return -1;
    }

    queue->capacidad = INITIAL_SENTENCE_QUEUE_CAPACITY;
    queue->cantidad = 0;
    queue->frente = 0;
    queue->mutexInicializado = false;
    queue->oraciones = malloc(sizeof(char *) * queue->capacidad);

    if (queue->oraciones == NULL)
    {
        queue->capacidad = 0;
        return -1;
    }

    if (pthread_mutex_init(&queue->mutex, NULL) != 0)
    {
        perror("pthread_mutex_init");
        free(queue->oraciones);
        queue->oraciones = NULL;
        queue->capacidad = 0;
        return -1;
    }

    queue->mutexInicializado = true;
    return 0;
}

void liberarSentenceQueue(SentenceQueue *queue)
{
    if (queue == NULL)
    {
        return;
    }

    if (queue->mutexInicializado)
    {
        pthread_mutex_lock(&queue->mutex);
    }

    for (int i = 0; i < queue->cantidad; i++)
    {
        int indice = (queue->frente + i) % queue->capacidad;
        free(queue->oraciones[indice]);
    }

    free(queue->oraciones);
    queue->oraciones = NULL;
    queue->capacidad = 0;
    queue->cantidad = 0;
    queue->frente = 0;

    if (queue->mutexInicializado)
    {
        pthread_mutex_unlock(&queue->mutex);
        pthread_mutex_destroy(&queue->mutex);
        queue->mutexInicializado = false;
    }
}

int encolarOracion(SentenceQueue *queue, const char *oracion)
{
    char *copia;

    if (queue == NULL || oracion == NULL)
    {
        return -1;
    }

    copia = copiarOracion(oracion);
    if (copia == NULL)
    {
        return -1;
    }

    if (queue->mutexInicializado)
    {
        pthread_mutex_lock(&queue->mutex);
    }

    if (queue->cantidad == queue->capacidad &&
        aumentarCapacidad(queue) == -1)
    {
        if (queue->mutexInicializado)
        {
            pthread_mutex_unlock(&queue->mutex);
        }

        free(copia);
        return -1;
    }

    int indice = (queue->frente + queue->cantidad) % queue->capacidad;

    queue->oraciones[indice] = copia;
    queue->cantidad++;

    if (queue->mutexInicializado)
    {
        pthread_mutex_unlock(&queue->mutex);
    }

    return 0;
}

int cantidadOraciones(const SentenceQueue *queue)
{
    int cantidad;

    if (queue == NULL)
    {
        return 0;
    }

    if (queue->mutexInicializado)
    {
        pthread_mutex_lock((pthread_mutex_t *)&queue->mutex);
    }

    cantidad = queue->cantidad;

    if (queue->mutexInicializado)
    {
        pthread_mutex_unlock((pthread_mutex_t *)&queue->mutex);
    }

    return cantidad;
}

int extraerOraciones(SentenceQueue *queue,
                     char **destino,
                     int maximo)
{
    int extraidas = 0;

    if (queue == NULL || destino == NULL || maximo <= 0)
    {
        return 0;
    }

    if (queue->mutexInicializado)
    {
        pthread_mutex_lock(&queue->mutex);
    }

    while (extraidas < maximo && queue->cantidad > 0)
    {
        destino[extraidas] = queue->oraciones[queue->frente];
        queue->oraciones[queue->frente] = NULL;
        queue->frente = (queue->frente + 1) % queue->capacidad;
        queue->cantidad--;
        extraidas++;
    }

    if (queue->mutexInicializado)
    {
        pthread_mutex_unlock(&queue->mutex);
    }

    return extraidas;
}
