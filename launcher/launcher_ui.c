#include <stdio.h>
#include <signal.h>
#include <unistd.h>

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

void ejecutarLauncher(LauncherContext *context)
{
    int opcion;

    do
    {
        actualizarEstados(&context->processManager);

        printf("\n===== AGENTIC OS =====\n");
        printf("1. Crear Window\n");
        printf("2. Cerrar todas las ventanas\n");
        printf("3. Mostrar procesos\n");
        printf("4. Salir\n");
        printf("> ");

        scanf("%d", &opcion);

        switch(opcion)
        {
            case 1:
            {
                int puerto = context->processManager.siguientePuerto++;

                if (context->socketIALearner != -1 &&
                    enviarComando(context, CMD_OPEN_WINDOW, puerto) == -1)
                {
                    printf("No fue posible abrir el puerto de la ventana.\n");
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