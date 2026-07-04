#ifndef SERVER_H
#define SERVER_H

#include "server_context.h"

int iniciarServidor(int puerto);
void aceptarClientes(int server_fd, ServerContext *contexto);

#endif // SERVER_H