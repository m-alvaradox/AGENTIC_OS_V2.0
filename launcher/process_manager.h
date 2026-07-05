#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <sys/types.h>

typedef enum {
    PROCESS_RUNNING,
    PROCESS_FINISHED
} ProcessState;

typedef struct
{
    int id;
    pid_t pid;
    ProcessState estado;
} ProcessInfo;

typedef struct
{
    ProcessInfo *procesos;

    int cantidad;

    int capacidad;

    int siguienteID;
    
} ProcessManager;

void inicializarProcessManager(ProcessManager *manager);

void liberarProcessManager(ProcessManager *manager);

int crearProcesoWindow(ProcessManager *manager);

void actualizarEstados(ProcessManager *manager);

void mostrarProcesos(const ProcessManager *manager);

#endif