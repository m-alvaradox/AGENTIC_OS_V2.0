#ifndef LAUNCHER_CONTEXT_H
#define LAUNCHER_CONTEXT_H

#include "process_manager.h"

typedef struct 
{
    ProcessManager processManager;
    int socketIALearner;

} LauncherContext;


#endif