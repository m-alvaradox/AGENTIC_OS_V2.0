#ifndef SERVER_CONTEXT_H
#define SERVER_CONTEXT_H

#include <pthread.h>

#include "config.h"
#include "dictionary.h"
#include "thread_manager.h"

typedef struct {
    Dictionary *correo;
    Dictionary *articulo;
    Dictionary *reporte;

    ThreadManager threadManager;

    int server_fd;
    int detectionThreads;
    int cpuCount;
    int siguientePuertoVentana;
    pthread_mutex_t puertoMutex;
    
} ServerContext;

#endif
