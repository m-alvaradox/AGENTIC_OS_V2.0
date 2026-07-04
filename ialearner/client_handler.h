#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <stddef.h>
#include "server_context.h"

typedef struct
{
    int client_fd;
    char *documento;
    size_t longitud; // tipo estandar de C para representar tamaños de memoria
    size_t capacidad;
    ServerContext *contexto;
} ClientInfo;

void *atenderCliente(void *arg);
void liberarCliente(ClientInfo *info);

int agregarCaracter(ClientInfo *info, char letra);

void procesarDocumento(ClientInfo *info);

#endif