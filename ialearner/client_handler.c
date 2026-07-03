#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

#include "client_handler.h"

void *atenderCliente(void *arg)
{
    ClientInfo *info = (ClientInfo *)arg;

    char letra;

    while (recv(info->client_fd,
                &letra,
                sizeof(char),
                0) > 0)
    {
        if (info->longitud == info->capacidad)
        {
            size_t nuevaCapacidad;

            if (info->capacidad == 0)
            {
                nuevaCapacidad = 256;
            }
            else
            {
                nuevaCapacidad = info->capacidad * 2;
            }

            // secure realloc

            char *temp = realloc(info->documento, nuevaCapacidad);

            if (temp == NULL)
            {
                perror("realloc");

                free(info->documento);
                close(info->client_fd);
                free(info);

                pthread_exit(NULL);
            }

            info->documento = temp;
            info->capacidad = nuevaCapacidad;
        }
        // Save word
        info->documento[info->longitud] = letra;
        info->longitud++;
    }
}

void liberarCliente(ClientInfo *info)
{
    if (info == NULL)
    {
        return;
    }

    free(info->documento);

    close(info->client_fd);

    free(info);
}

int agregarCaracter(ClientInfo *info, char letra)
{
}