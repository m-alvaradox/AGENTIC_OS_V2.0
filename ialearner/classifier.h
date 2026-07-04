#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include "dictionary.h"

void clasificarDocumento(const char *documento,
                         const Dictionary *correo,
                         const Dictionary *articulo,
                         const Dictionary *reporte);

#endif