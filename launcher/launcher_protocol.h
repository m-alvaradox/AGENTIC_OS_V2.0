#ifndef LAUNCHER_PROTOCOL_H
#define LAUNCHER_PROTOCOL_H

#include "launcher_context.h"
#include "protocol.h"

int conectarIALearner(LauncherContext *context);

void desconectarIALearner(LauncherContext *context);

int enviarComando(
    LauncherContext *context,
    ControlCommand comando
);

int recibirContexto(
    LauncherContext *context,
    UserContext *contexto);

#endif