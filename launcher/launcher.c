#include <stdio.h>

#include "launcher_ui.h"
#include "process_manager.h"
#include "launcher_protocol.h"

int main (void)
{
    ProcessManager manager;

    inicializarProcessManager(&manager);

    if(conectarIALearner() == -1)
    {
        printf("No fue posible conectarse con IALearner.\n");
        liberarProcessManager(&manager);
        return 1;
    }

    ejecutarLauncher(&manager);

    desconectarIALearner();

    liberarProcessManager(&manager);

    return 0;
}