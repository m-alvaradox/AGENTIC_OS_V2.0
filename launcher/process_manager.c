#include <stdlib.h>

#include "process_manager.h"

#define INITIAL_CAPACITY 10

void inicializarProcessManager(ProcessManager *manager)
{
    manager->cantidad = 0;
    manager->capacidad = INITIAL_CAPACITY;

    manager->procesos = malloc(
        sizeof(ProcessInfo) * manager->capacidad);
}

void liberarProcessManager(ProcessManager *manager)
{
    free(manager->procesos);
}