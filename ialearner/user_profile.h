#ifndef USER_PROFILE_H
#define USER_PROFILE_H

#include "classification.h"
#include "protocol.h"

typedef struct {
    int correos;
    int articulos;
    int reportes;
} UserProfile;

void inicializarPerfil(UserProfile *perfil);

void registrarDocumento(UserProfile *perfil,
                        DocumentClass clase);

UserType clasificarUsuario(const UserProfile *perfil);

void imprimirPerfilUsuario(UserType tipo);

#endif