#include <stdio.h>

#include "launcher_ui.h"
#include "process_manager.h"
#include "launcher_protocol.h"

int main (void)
{
    LauncherContext context;

   inicializarProcessManager(&context.processManager);

    if(conectarIALearner(&context) == -1)
    {
        printf("No fue posible conectarse con IALearner.\n");
        liberarProcessManager(&context.processManager);
        return 1;
    }

    ejecutarLauncher(&context);

    desconectarIALearner(&context);

    liberarProcessManager(&context.processManager);

    return 0;
}