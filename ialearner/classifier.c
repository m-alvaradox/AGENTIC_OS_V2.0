#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classifier.h"
#include "config.h"
#include "utils.h"

/*
static void convertirMinusculas(char *texto)
{

    for (int i = 0; texto[i] != '\0'; i++)
    {
        texto[i] = tolower((unsigned char)texto[i]);
    }
} */

ClassificationResult clasificarDocumento(const char *documento,
                                         const Dictionary *correo,
                                         const Dictionary *articulo,
                                         const Dictionary *reporte)
{
    ClassificationResult resultado = {
        .clase = DOC_SIN_CLASIFICAR,
        .coincidenciasCorreo = 0,
        .coincidenciasArticulo = 0,
        .coincidenciasReporte = 0};

    if (documento == NULL ||
        correo == NULL ||
        articulo == NULL ||
        reporte == NULL)
    {
        return resultado;
    }

    char *copia = malloc(strlen(documento) + 1);

    if (copia == NULL)
    {
        perror("malloc");
        return resultado;
    }

    strcpy(copia, documento);

    convertirMinusculas(copia);

    // Tokenizacion
    char *token = strtok(copia, TOKEN_DELIMITERS);

    while (token != NULL)
    {
        // caso word se encuentra en mas diccionarios

        if (diccionarioContiene(correo, token))
        {
            resultado.coincidenciasCorreo++;
        }

        if (diccionarioContiene(articulo, token))
        {
            resultado.coincidenciasArticulo++;
        }

        if (diccionarioContiene(reporte, token))
        {
            resultado.coincidenciasReporte++;
        }

        token = strtok(NULL, TOKEN_DELIMITERS);
    }

    if (resultado.coincidenciasCorreo < 3)
    {
        resultado.coincidenciasCorreo = 0;
    }

    if (resultado.coincidenciasArticulo < 3)
    {
        resultado.coincidenciasArticulo = 0;
    }

    if (resultado.coincidenciasReporte < 3)
    {
        resultado.coincidenciasReporte = 0;
    }

    int mayor = 0;

    if (resultado.coincidenciasCorreo > mayor)
    {
        mayor = resultado.coincidenciasCorreo;
        resultado.clase = DOC_CORREO;
    }

    if (resultado.coincidenciasArticulo > mayor)
    {
        mayor = resultado.coincidenciasArticulo;
        resultado.clase = DOC_ARTICULO;
    }

    if (resultado.coincidenciasReporte > mayor)
    {
        mayor = resultado.coincidenciasReporte;
        resultado.clase = DOC_REPORTE;
    }

    free(copia);
    return resultado;
}

void imprimirClasificacion(const ClassificationResult *resultado)
{
    printf("\n====== CLASIFICACIÓN DEL DOCUMENTO =====\n");
    printf("Palabras correo: %d\n", resultado->coincidenciasCorreo);
    printf("Parlabras articulo: %d\n", resultado->coincidenciasArticulo);
    printf("Palabras reporte: %d\n\n", resultado->coincidenciasReporte);

    switch (resultado->clase)
    {
    case DOC_CORREO:
        printf("Clase: Correo electrónico\n");
        break;

    case DOC_ARTICULO:
        printf("Clase: Artículo científico\n");
        break;

    case DOC_REPORTE:
        printf("Clase: Reporte\n");
        break;

    default:
        printf("Clase: Sin clasificar\n");
    }

    printf("========================================\n");
}
