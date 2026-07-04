#ifndef NETWORK_H
#define NETWORK_H

int conectarServidor(void);
void cerrarConexion(int socket_fd);

int enviarCaracter(int socket_fd, char caracter);

#endif