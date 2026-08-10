#ifndef SENTENCE_QUEUE_H
#define SENTENCE_QUEUE_H

#include <stdbool.h>
#include <pthread.h>

typedef struct
{
    char **oraciones;
    int capacidad;
    int cantidad;
    int frente;
    bool mutexInicializado;
    bool condInicializada;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} SentenceQueue;

int inicializarSentenceQueue(SentenceQueue *queue);

void liberarSentenceQueue(SentenceQueue *queue);

int encolarOracion(SentenceQueue *queue, const char *oracion);

int cantidadOraciones(const SentenceQueue *queue);

int extraerOraciones(SentenceQueue *queue,
                     char **destino,
                     int maximo);

void despertarSentenceQueue(SentenceQueue *queue);

#endif
