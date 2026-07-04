#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <sys/types.h>

typedef enum {
    PROCESS_RUNNING,
    PROCESS_FINISHED
} ProcessState;

typedef struct
{
    pid_t pid;
    ProcessState estado;
} ProcessInfo;

typedef struct
{
    ProcessInfo *procesos;

    int cantidad;
    int capacidad;
} ProcessManager;

void inicializarProcessManager(ProcessManager *manager);

void liberarProcessManager(ProcessManager *manager);


#endif