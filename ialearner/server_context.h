#ifndef SERVER_CONTEXT_H
#define SERVER_CONTEXT_H

#include "dictionary.h"
#include "user_profile.h"

typedef struct {
    Dictionary *correo;
    Dictionary *articulo;
    Dictionary *reporte;

    UserProfile perfil;
    
} ServerContext;

#endif