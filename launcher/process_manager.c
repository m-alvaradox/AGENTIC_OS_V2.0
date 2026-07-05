#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>

#include "process_manager.h"
#include "config.h"

void inicializarProcessManager(ProcessManager *manager)
{
    if (manager == NULL)
    {
        return;
    }

    manager->cantidad = 0;
    manager->capacidad = INITIAL_CAPACITY;

    manager->procesos = malloc(
        sizeof(ProcessInfo) * manager->capacidad);

    if (manager->procesos == NULL)
    {
        manager->capacidad = 0;
    }

    manager->siguienteID = 1;
    manager->siguientePuerto = WINDOW_BASE_PORT;
}

void liberarProcessManager(ProcessManager *manager)
{
    if (manager == NULL)
    {
        return;
    }

    for (int i = 0; i < manager->cantidad; i++)
    {
        pid_t pid = manager->procesos[i].pid;

        if (manager->procesos[i].estado == PROCESS_RUNNING)
        {
            kill(pid, SIGTERM);
            waitpid(pid, NULL, 0);
        }
        else
        {
            waitpid(pid, NULL, WNOHANG);
        }
    }

    free(manager->procesos);
    manager->procesos = NULL;
    manager->cantidad = 0;
    manager->capacidad = 0;
    manager->siguienteID = 1;
    manager->siguientePuerto = WINDOW_BASE_PORT;
}

static int agregarProceso(ProcessManager *manager, pid_t pid, int puerto)
{
    if (manager == NULL)
    {
        return -1;
    }

    if (manager->procesos == NULL && manager->capacidad != 0)
    {
        return -1;
    }

    if (manager->cantidad == manager->capacidad)
    {
        if (manager->capacidad == 0)
        {
            manager->capacidad = INITIAL_CAPACITY;
        }

        manager->capacidad *= 2;

        ProcessInfo *temp = realloc(manager->procesos,
                                    sizeof(ProcessInfo) * manager->capacidad);

        if (temp == NULL)
        {
            return -1;
        }

        manager->procesos = temp;
    }

    manager->procesos[manager->cantidad].id = manager->siguienteID++;
    manager->procesos[manager->cantidad].pid = pid;
    manager->procesos[manager->cantidad].puerto = puerto;
    manager->procesos[manager->cantidad].estado = PROCESS_RUNNING;

    manager->cantidad++;

    return 0;
}

int crearProcesoWindow(ProcessManager *manager, int puerto)
{
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return -1;
    }

    if (pid == 0)
    {
        char puertoStr[16];
        snprintf(puertoStr, sizeof(puertoStr), "%d", puerto);

        execl(WINDOW_EXECUTABLE,
              "window",
              puertoStr,
              (char *)NULL);

        perror("execl");

        exit(EXIT_FAILURE);
    }

    if (agregarProceso(manager, pid, puerto) == -1)
    {
        kill(pid, SIGTERM);
        waitpid(pid, NULL, 0);
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
    printf("\n============= Procesos ===============\n");

    for (int i = 0; i < manager->cantidad; i++)
    {
        printf("ID: %d   PID: %d   Puerto: %d   Estado: %s\n",
               manager->procesos[i].id,
               manager->procesos[i].pid,
               manager->procesos[i].puerto,
               manager->procesos[i].estado == PROCESS_RUNNING
                    ? "RUNNING"
                    : "FINISHED");
    }

    printf("======================================\n");
}