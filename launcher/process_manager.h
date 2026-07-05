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
    int puerto;
    ProcessState estado;
} ProcessInfo;

typedef struct
{
    ProcessInfo *procesos;

    int cantidad;

    int capacidad;

    int siguienteID;
    int siguientePuerto;
    
} ProcessManager;

void inicializarProcessManager(ProcessManager *manager);

void liberarProcessManager(ProcessManager *manager);

int crearProcesoWindow(ProcessManager *manager, int puerto);

void actualizarEstados(ProcessManager *manager);

void mostrarProcesos(const ProcessManager *manager);

#endif