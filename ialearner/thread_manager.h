#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <pthread.h>

typedef struct
{

    pthread_t *hilos;

    int cantidad;
    int capacidad;

} ThreadManager;

void inicializarThreadManager(ThreadManager *manager);

void liberarThreadManager(ThreadManager *manager);

int agregarThread(ThreadManager *manager, pthread_t thread);

void esperarThreads(ThreadManager *manager);

#endif