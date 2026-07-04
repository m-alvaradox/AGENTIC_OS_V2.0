#ifndef SERVER_CONTEXT_H
#define SERVER_CONTEXT_H

#include "dictionary.h"

typedef struct {
    Dictionary *correo;
    Dictionary *articulo;
    Dictionary *reporte;
} ServerContext;

#endif