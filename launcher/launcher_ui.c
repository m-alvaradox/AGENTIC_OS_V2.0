#include <stdio.h>

#include "launcher_ui.h"

void ejecutarLauncher(LauncherContext *context)
{
    int opcion;

    do
    {
        actualizarEstados(&context->processManager);

        printf("\n===== AGENTIC OS =====\n");
        printf("1. Crear Window\n");
        printf("2. Mostrar procesos\n");
        printf("3. Salir\n");
        printf("> ");

        scanf("%d", &opcion);

        switch(opcion)
        {
            case 1:
                if (crearProcesoWindow(&context->processManager) == -1) {
                    printf("No fue posible crear la ventana.\n");
                }
                break;

            case 2:
                mostrarProcesos(&context->processManager);
                break;

            case 3:
                printf("Finalizando...\n");
                break;

            default:
                printf("Opción inválida.\n");
        }

    } while(opcion != 3);
}