#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "launcher_protocol.h"
#include "config.h"

int conectarIALearner(LauncherContext *context)
{
    struct sockaddr_in servidor;

    context->socketIALearner = socket(AF_INET,
                                      SOCK_STREAM,
                                      0);

    if (context->socketIALearner == -1)
    {
        perror("socket");

        return -1;
    }

    memset(&servidor, 0, sizeof(servidor));

    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(CONTROL_SERVICE_PORT);
    servidor.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(context->socketIALearner,
                (struct sockaddr *)&servidor,
                sizeof(servidor)) == -1)
    {
        perror("connect");

        close(context->socketIALearner);

        return -1;
    }

    return 0;
}

void desconectarIALearner(LauncherContext *context)
{
    if (context == NULL || context->socketIALearner == -1)
    {
        return;
    }

    close(context->socketIALearner);
    context->socketIALearner = -1;
}

int enviarComando(
        LauncherContext *context,
        ControlCommand comando,
        int puerto)
{
    if (context == NULL)
    {
        return -1;
    }

    ControlMessage mensaje;
    mensaje.comando = comando;
    mensaje.puerto = puerto;

    ssize_t enviados = send(
            context->socketIALearner,
            &mensaje,
            sizeof(ControlMessage),
            0);

    if (enviados == -1)
    {
        perror("send");
        return -1;
    }

    return 0;
}

int recibirContexto(
        LauncherContext *context,
        UserContext *contexto)
{
    if (context == NULL || contexto == NULL)
    {
        return -1;
    }

    ssize_t recibidos = recv(
            context->socketIALearner,
            contexto,
            sizeof(UserContext),
            0);

    if (recibidos == -1)
    {
        perror("recv");
        return -1;
    }

    return (int)recibidos;
}