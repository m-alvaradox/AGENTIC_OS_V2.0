#include <stdlib.h>
#include <stdio.h>

#include "config.h"
#include "thread_manager.h"

void inicializarThreadManager(ThreadManager *manager)
{
    if (manager == NULL)
    {
        return;
    }

    manager->cantidad = 0;
    manager->capacidad = INITIAL_THREAD_CAPACITY;
    manager->mutexInicializado = false;

    manager->hilos = malloc(
        sizeof(pthread_t) * manager->capacidad);

    if (manager->hilos == NULL)
    {
        manager->capacidad = 0;
    }

    if (pthread_mutex_init(&manager->mutex, NULL) != 0)
    {
        perror("pthread_mutex_init");
        free(manager->hilos);
        manager->hilos = NULL;
        manager->capacidad = 0;
        return;
    }

    manager->mutexInicializado = true;
}

void liberarThreadManager(ThreadManager *manager)
{
    if (manager == NULL)
    {
        return;
    }

    if (manager->mutexInicializado)
    {
        pthread_mutex_destroy(&manager->mutex);
        manager->mutexInicializado = false;
    }

    free(manager->hilos);

    manager->hilos = NULL;
    manager->cantidad = 0;
    manager->capacidad = 0;
}

int agregarThread(ThreadManager *manager,
                  pthread_t thread)
{
    if (manager == NULL)
    {
        return -1;
    }

    if (manager->mutexInicializado)
    {
        pthread_mutex_lock(&manager->mutex);
    }

    if (manager->cantidad == manager->capacidad)
    {
        if (manager->capacidad == 0)
        {
            manager->capacidad = INITIAL_THREAD_CAPACITY;
        }

        manager->capacidad *= 2;

        pthread_t *temp =
            realloc(manager->hilos,
                    sizeof(pthread_t) *
                    manager->capacidad);

        if (temp == NULL)
        {
            if (manager->mutexInicializado)
            {
                pthread_mutex_unlock(&manager->mutex);
            }

            return -1;
        }

        manager->hilos = temp;
    }

    manager->hilos[manager->cantidad++] =
        thread;

    if (manager->mutexInicializado)
    {
        pthread_mutex_unlock(&manager->mutex);
    }

    return 0;
}

void esperarThreads(ThreadManager *manager)
{
    if (manager == NULL || manager->hilos == NULL)
    {
        return;
    }

    if (manager->mutexInicializado)
    {
        pthread_mutex_lock(&manager->mutex);
    }

    int cantidad = manager->cantidad;

    if (manager->mutexInicializado)
    {
        pthread_mutex_unlock(&manager->mutex);
    }

    for (int i = 0; i < cantidad; i++)
    {
        pthread_cancel(manager->hilos[i]);
        pthread_join(manager->hilos[i], NULL);
    }
}

void unirThreads(ThreadManager *manager)
{
    if (manager == NULL || manager->hilos == NULL)
    {
        return;
    }

    if (manager->mutexInicializado)
    {
        pthread_mutex_lock(&manager->mutex);
    }

    int cantidad = manager->cantidad;

    if (manager->mutexInicializado)
    {
        pthread_mutex_unlock(&manager->mutex);
    }

    for (int i = 0; i < cantidad; i++)
    {
        pthread_join(manager->hilos[i], NULL);
    }
}
