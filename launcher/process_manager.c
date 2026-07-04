#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include "process_manager.h"
#include "config.h"

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

static int agregarProceso(ProcessManager *manager, pid_t pid)
{
    if (manager->cantidad == manager->capacidad)
    {
        manager->capacidad *= 2;

        ProcessInfo *temp = realloc(manager->procesos,
                                    sizeof(ProcessInfo) * manager->capacidad);

        if (temp == NULL)
        {
            return -1;
        }

        manager->procesos = temp;
    }

    manager->procesos[manager->cantidad].pid = pid;
    manager->procesos[manager->cantidad].estado = PROCESS_RUNNING;

    manager->cantidad++;

    return 0;
}

int crearProcesoWindow(ProcessManager *manager)
{

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return -1;
    }

    if (pid == 0)
    {
        execl(WINDOW_EXECUTABLE,
              "window",
              (char *)NULL);

        perror("execl");

        exit(EXIT_FAILURE);
    }

    if (agregarProceso(manager, pid) == -1)
    {
        return -1;
    }

    return 0;
}

void actualizarEstados(ProcessManager *manager)
{
    int status;

    for (int i = 0; i < manager->cantidad; i++)
    {
        if (manager->procesos[i].estado == PROCESS_FINISHED)
        {
            continue;
        }
        // waitpid, wnohang flag para consultar estado del pid sin detener el programa (sin bloqueos)
        pid_t resultado = waitpid(manager->procesos[i].pid,
                                  &status,
                                  WNOHANG);

        if (resultado == manager->procesos[i].pid)
        {
            manager->procesos[i].estado = PROCESS_FINISHED;
        }
    }
}

void mostrarProcesos(const ProcessManager *manager)
{
    printf("\n========== Procesos ==========\n");

    for (int i = 0; i < manager->cantidad; i++)
    {
        printf("PID: %d   Estado: %s\n",
               manager->procesos[i].pid,
               manager->procesos[i].estado == PROCESS_RUNNING
                    ? "RUNNING"
                    : "FINISHED");
    }

    printf("==============================\n");
}