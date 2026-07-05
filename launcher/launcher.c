#include <stdio.h>
#include <signal.h>

#include "launcher_ui.h"
#include "process_manager.h"
#include "launcher_protocol.h"
#include "user_profile.h"

static void inicializarLauncherContext(LauncherContext *context)
{

    inicializarProcessManager(&context->processManager);
    context->socketIALearner = -1;
    context->ialearnerDisponible = 0;
}

static void liberarLauncherContext(LauncherContext *context)
{
    liberarProcessManager(&context->processManager);
}

int main(void)
{
    LauncherContext context;

    signal(SIGPIPE, SIG_IGN);

    inicializarLauncherContext(&context);

    if (conectarIALearner(&context) == -1)
    {
        printf("No fue posible conectarse con IALearner.\n");

        liberarLauncherContext(&context);

        return 1;
    }

    context.ialearnerDisponible = 1;

    enviarComando(&context, CMD_START, 0);

    ejecutarLauncher(&context);

    UserContext usuario;

    if (!context.ialearnerDisponible)
    {
        desconectarIALearner(&context);
        liberarLauncherContext(&context);

        return 1;
    }

    if (enviarComando(&context, CMD_END, 0) == -1)
    {
        printf("No fue posible enviar el comando de finalizacion.\n");

        desconectarIALearner(&context);
        liberarLauncherContext(&context);

        return 1;
    }

    if (recibirContexto(&context, &usuario) <= 0)
    {
        printf("No fue posible recibir el contexto del usuario.\n");

        desconectarIALearner(&context);
        liberarLauncherContext(&context);

        return 1;
    }

    imprimirPerfilUsuario(usuario.tipo);

    desconectarIALearner(&context);

    liberarLauncherContext(&context);

    return 0;
}
