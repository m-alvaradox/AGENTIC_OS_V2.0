#ifndef SERVER_CONTEXT_H
#define SERVER_CONTEXT_H

#include <stdbool.h>

#include "config.h"
#include "dictionary.h"
#include "user_profile.h"
#include "thread_manager.h"

typedef struct {
    Dictionary *correo;
    Dictionary *articulo;
    Dictionary *reporte;

    UserProfile perfil;

    ThreadManager threadManager;

    int launcherSocket;
    int server_fd;
    int window_server_fds[MAX_WINDOW_SERVERS];
    int window_ports[MAX_WINDOW_SERVERS];
    int window_count;

    bool sessionActiva;
    
} ServerContext;

#endif