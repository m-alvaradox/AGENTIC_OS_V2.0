#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "network.h"
#include "config.h"

int conectarServidor(void)
{
    int socket_fd;
    struct sockaddr_in servidor;

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd == -1)
    {
        perror("socket cliente");
        return -1;
    }

    memset(&servidor, 0, sizeof(servidor)); // limpiar campos de la estructura

    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(SERVER_PORT);
    servidor.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to the server

    if (connect(socket_fd,
                (struct sockaddr *)&servidor,
                sizeof(servidor)) == -1)
    {
        perror("Error al conectar con IA Learner");
        close(socket_fd);
        return -1;
    }

    return socket_fd;
}

void cerrarConexion(int socket_fd)
{
    close(socket_fd);
}

int enviarCaracter(int socket_fd, char caracter) 
{
    int enviados = send(socket_fd, &caracter, 1, 0);

    if (enviados == -1)
    {
        perror("Error al enviar caracter");
        return -1;
    }

    return 0;

}
