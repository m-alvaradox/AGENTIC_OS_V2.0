#include "config.h"
#include "thread.manager.h"

void inicializarThreadManager(ThreadManager *manager)
{
    manager->cantidad = 0;
    manager->capacidad = INITIAL_THREAD_CAPACITY;

    manager->hilos = malloc(
        sizeof(pthread_t) * manager->capacidad);
}

void liberarThreadManager(ThreadManager *manager)
{
    free(manager->hilos);
}

int agregarThread(ThreadManager *manager,
                  pthread_t thread)
{
    if (manager->cantidad == manager->capacidad)
    {
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
    for (int i = 0; i < manager->cantidad; i++)
    {
        // wait for termination
        pthread_join(manager->hilos[i], NULL);
    }
}