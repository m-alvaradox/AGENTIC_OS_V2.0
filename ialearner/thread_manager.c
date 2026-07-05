#include <stdlib.h>

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

    manager->hilos = malloc(
        sizeof(pthread_t) * manager->capacidad);

    if (manager->hilos == NULL)
    {
        manager->capacidad = 0;
    }
}

void liberarThreadManager(ThreadManager *manager)
{
    if (manager == NULL)
    {
        return;
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
            return -1;
        }

        manager->hilos = temp;
    }

    manager->hilos[manager->cantidad++] =
        thread;

    return 0;
}

void esperarThreads(ThreadManager *manager)
{
    if (manager == NULL || manager->hilos == NULL)
    {
        return;
    }

    for (int i = 0; i < manager->cantidad; i++)
    {
        pthread_join(manager->hilos[i], NULL); // wait for termination
    }
}