#ifndef LAUNCHER_PROTOCOL_H
#define LAUNCHER_PROTOCOL_H

#include "launcher_context.h"

int conectarIALearner(LauncherContext *context);

void desconectarIALearner(LauncherContext *context);

int enviarStart(LauncherContext *context);

int enviarEnd(LauncherContext *context);

int recibirContexto(
    LauncherContext *context,
    char *buffer,
    size_t size
);


#endif