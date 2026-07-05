#ifndef SERVER_H
#define SERVER_H

#include "session_context.h"

typedef struct
{
    int server_fd;
    int puerto;
    SessionContext *session;
} WindowServiceArgs;

int iniciarServidor(int puerto);
void aceptarClientes(int server_fd, SessionContext *session);
void *aceptarClientesVentana(void *arg);

#endif // SERVER_H
