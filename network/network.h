#ifndef NETWORK_H
#define NETWORK_H

int conectarServidor(int puerto);
void cerrarConexion(int socket_fd);

int enviarCaracter(int socket_fd, char caracter);

#endif