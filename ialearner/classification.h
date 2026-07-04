#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H

typedef enum {
    DOC_SIN_CLASIFICAR = 0,
    DOC_CORREO,
    DOC_ARTICULO,
    DOC_REPORTE
} DocumentClass;

typedef struct
{
    DocumentClass clase;

    int coincidenciasCorreo;
    int coincidenciasArticulo;
    int coincidenciasReporte;

} ClassificationResult;

#endif