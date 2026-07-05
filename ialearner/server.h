#ifndef SERVER_H
#define SERVER_H

#include "server_context.h"

typedef struct
{
    int server_fd;
    int puerto;
    ServerContext *contexto;
} WindowServiceArgs;

int iniciarServidor(int puerto);
void aceptarClientes(int server_fd, ServerContext *contexto);
void *aceptarClientesVentana(void *arg);

#endif // SERVER_H