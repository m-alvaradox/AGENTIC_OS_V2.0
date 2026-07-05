#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include "launcher_ui.h"
#include "launcher_protocol.h"
#include "protocol.h"

static void cerrarTodasVentanas(LauncherContext *context)
{
    if (context == NULL)
    {
        return;
    }

    actualizarEstados(&context->processManager);

    for (int i = 0; i < context->processManager.cantidad; i++)
    {
        if (context->processManager.procesos[i].estado != PROCESS_RUNNING)
        {
            continue;
        }

        if (context->socketIALearner != -1)
        {
            enviarComando(context,
                          CMD_CLOSE_WINDOW,
                          context->processManager.procesos[i].puerto);
        }

        kill(context->processManager.procesos[i].pid, SIGTERM);
    }

    actualizarEstados(&context->processManager);
}

static void manejarIALearnerNoDisponible(LauncherContext *context)
{
    printf("IALearner no esta disponible. Cerrando ventanas locales...\n");
    cerrarTodasVentanas(context);
}

static int leerOpcionMenu(int *opcion)
{
    char linea[32];
    char *fin;
    long valor;

    if (opcion == NULL)
    {
        return -1;
    }

    if (fgets(linea, sizeof(linea), stdin) == NULL)
    {
        return -1;
    }

    errno = 0;
    valor = strtol(linea, &fin, 10);

    if (fin == linea || errno == ERANGE || valor < INT_MIN || valor > INT_MAX)
    {
        return -1;
    }

    while (*fin == ' ' || *fin == '\t')
    {
        fin++;
    }

    if (*fin != '\n' && *fin != '\0')
    {
        return -1;
    }

    *opcion = (int)valor;
    return 0;
}

void ejecutarLauncher(LauncherContext *context)
{
    int opcion = 0;

    do
    {
        actualizarEstados(&context->processManager);

        if (!context->ialearnerDisponible)
        {
            manejarIALearnerNoDisponible(context);
            break;
        }

        printf("\n===== AGENTIC OS =====\n");
        printf("1. Crear Window\n");
        printf("2. Cerrar todas las ventanas\n");
        printf("3. Mostrar procesos\n");
        printf("4. Salir\n");
        printf("> ");

        if (leerOpcionMenu(&opcion) == -1)
        {
            printf("Opción inválida.\n");
            continue;
        }

        switch(opcion)
        {
            case 1:
            {
                int puerto;

                if (context->socketIALearner == -1 ||
                    solicitarVentana(context, &puerto) == -1)
                {
                    printf("No fue posible abrir el puerto de la ventana.\n");
                    manejarIALearnerNoDisponible(context);
                    opcion = 4;
                    break;
                }

                if (crearProcesoWindow(&context->processManager, puerto) == -1)
                {
                    if (context->socketIALearner != -1)
                    {
                        enviarComando(context, CMD_CLOSE_WINDOW, puerto);
                    }

                    printf("No fue posible crear la ventana.\n");
                }
                break;
            }

            case 2:
                cerrarTodasVentanas(context);
                break;

            case 3:
                actualizarEstados(&context->processManager);
                mostrarProcesos(&context->processManager);
                break;

            case 4:
                printf("Finalizando...\n");
                cerrarTodasVentanas(context);
                break;

            default:
                printf("Opción inválida.\n");
        }

    } while(opcion != 4);
}
