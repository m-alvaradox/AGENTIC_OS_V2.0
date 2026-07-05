#include <stdio.h>

#include "launcher_ui.h"
#include "process_manager.h"
#include "launcher_protocol.h"

static void inicializarLauncherContext(LauncherContext *context)
{

    inicializarProcessManager(&context->processManager);
    context->socketIALearner = -1;
}

static void liberarLauncherContext(LauncherContext *context)
{
    liberarProcessManager(&context->processManager);
}

int main(void)
{
    LauncherContext context;

    inicializarLauncherContext(&context);

    if (conectarIALearner(&context) == -1)
    {
        printf("No fue posible conectarse con IALearner.\n");

        liberarLauncherContext(&context);

        return 1;
    }

    enviarComando(&context, CMD_START);

    ejecutarLauncher(&context);

    UserContext usuario;

    enviarComando(&context, CMD_END);

    recibirContexto(&context, &usuario);

    desconectarIALearner(&context);

    liberarLauncherContext(&context);

    return 0;
}