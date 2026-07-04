#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>

#include "client_handler.h"
#include "classifier.h"

void *atenderCliente(void *arg)
{
    ClientInfo *info = (ClientInfo *)arg;

    char letra;

    while (recv(info->client_fd,
                &letra,
                sizeof(char),
                0) > 0)
    {
        if (agregarCaracter(info, letra) == -1)
        {
            liberarCliente(info);
            return NULL;
        }

        printf("Recibido: %c\n", letra);
    }

    procesarDocumento(info);

    liberarCliente(info);

    return NULL;
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
    if (info == NULL)
    {
        return -1;
    }

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
            return -1;
        }

        info->documento = temp;
        info->capacidad = nuevaCapacidad;
    }

    // Save word
    info->documento[info->longitud] = letra;
    info->longitud++;
    return 0;
}

void procesarDocumento(ClientInfo *info)
{
    if (info == NULL)
    {
        return;
    }

    if (info->longitud == info->capacidad)
    { // Para asegurar espacio para el carácter nulo \0... 1 byte extra
        char *temp = realloc(info->documento,
                             info->capacidad + 1);

        if (temp == NULL)
        {
            perror("realloc");
            return;
        }

        info->documento = temp;
        info->capacidad += 1;
    }

    info->documento[info->longitud] = '\0'; // Null-terminate the string

    // Por ahora, simplemente imprimimos el documento recibido

    printf("\n========== DOCUMENTO RECIBIDO ==========\n");
    printf("%s\n", info->documento);
    printf("==========================================\n");

    ClassificationResult resultado;

    resultado = clasificarDocumento(info->documento,
                        info->contexto->correo,
                        info->contexto->articulo,
                        info->contexto->reporte);

    imprimirClasificacion(&resultado);
}