#include <stdio.h>

#include "launcher_ui.h"

void ejecutarLauncher(ProcessManager *manager)
{
    int opcion;

    do
    {
        printf("\n===== AGENTIC OS =====\n");
        printf("1. Crear Window\n");
        printf("2. Mostrar procesos\n");
        printf("3. Salir\n");
        printf("> ");

        scanf("%d", &opcion);

        switch(opcion)
        {
            case 1:
                printf("Crear Window\n");
                break;

            case 2:
                printf("Mostrar procesos\n");
                break;

            case 3:
                printf("Finalizando...\n");
                break;

            default:
                printf("Opción inválida.\n");
        }

    } while(opcion != 3);
}