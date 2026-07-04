#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "server_context.h"
#include "dictionary.h"
#include "config.h"

int main(void)
{
    int server_fd;

    server_fd = iniciarServidor(SERVER_PORT);

    if (server_fd == -1)
    {
        return EXIT_FAILURE;
    }

    // Definir contexto

    ServerContext contexto = {
        .correo = cargarDiccionario(DICTIONARY_DIRECTORY EMAIL_DICTIONARY_FILE, EMAIL_CLASS_NAME),
        .articulo = cargarDiccionario(DICTIONARY_DIRECTORY ARTICLE_DICTIONARY_FILE, ARTICLE_CLASS_NAME),
        .reporte = cargarDiccionario(DICTIONARY_DIRECTORY REPORT_DICTIONARY_FILE, REPORT_CLASS_NAME),
    };

    if (contexto.correo == NULL ||
        contexto.articulo == NULL ||
        contexto.reporte == NULL)
    {
        fprintf(stderr, "Error al cargar los diccionarios\n");

        liberarDiccionario(contexto.correo);
        liberarDiccionario(contexto.articulo);
        liberarDiccionario(contexto.reporte);
    }

    printf("=== IA Learner Data Center ===\n");
    printf("Servidor del Data Center iniciado...\n");
    printf("Esperando conexiones...\n\n");

    aceptarClientes(server_fd, &contexto);

    liberarDiccionario(contexto.correo);
    liberarDiccionario(contexto.articulo);
    liberarDiccionario(contexto.reporte);

    return EXIT_SUCCESS;
}