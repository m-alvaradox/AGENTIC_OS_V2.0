#include <stdio.h>
#include <stdlib.h>

#include "server.h"

int main(void)
{
    int server_fd;

    server_fd = iniciarServidor(5000);

    if (server_fd == -1)
    {
        return EXIT_FAILURE;
    }

    printf("Servidor Data center iniciado.\n");
    printf("Esperando conexiones...\n");

    aceptarClientes(server_fd);

    return EXIT_SUCCESS;
}