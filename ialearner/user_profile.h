#ifndef USER_PROFILE_H
#define USER_PROFILE_H

#include "classification.h"

typedef struct {
    int correos;
    int articulos;
    int reportes;
} UserProfile;

typedef enum {
    USER_NO_DETECTADO = 0,
    USER_ADMINISTRATIVO,
    USER_TECNICO,
    USER_PROFESOR,
    USER_ESTUDIANTE
} UserType;

void inicializarPerfil(UserProfile *perfil);

void registrarDocumento(UserProfile *perfil,
                        DocumentClass clase);

UserType clasificarUsuario(const UserProfile *perfil);

void imprimirPerfilUsuario(UserType tipo);

#endif