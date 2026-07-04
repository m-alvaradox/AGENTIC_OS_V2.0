#ifndef CLASSIFIER_H
#define CLASSIFIER_H

#include "dictionary.h"
#include "classification.h"

ClassificationResult clasificarDocumento(const char *documento,
                         const Dictionary *correo,
                         const Dictionary *articulo,
                         const Dictionary *reporte);

void imprimirClasificacion(const ClassificationResult *resultado);

#endif