#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "classifier.h"
#include "dictionary.h"
#include "config.h"

void clasificarDocumento(const char *documento,
                         const Dictionary *correo,
                         const Dictionary *articulo,
                         const Dictionary *reporte)
{
    printf("\n========== CLASIFICACIÓN DEL DOCUMENTO ==========\n");

    if (documento == NULL ||
        correo == NULL ||
        articulo == NULL ||
        reporte == NULL)
    {
        return;
    }

    char *copia = malloc(strlen(documento) + 1);

    if (copia == NULL)
    {
        perror("malloc");
        return;
    }

    strcpy(copia, documento);

    convertirMinusculas(copia);

    int count_words_correo = 0;
    int count_words_articulo = 0;
    int count_words_reporte = 0;

    // Tokenizacion
    char *token = strtok(copia, TOKEN_DELIMITERS);

    while (token != NULL)
    {
        // caso word se encuentra en mas diccionarios

        if (existePalabra(token, correo))
        {
            count_words_correo++;
        }

        if (existePalabra(token, articulo))
        {
            count_words_articulo++;
        }

        if (existePalabra(token, reporte))
        {
            count_words_reporte++;
        }

        token = strtok(NULL, TOKEN_DELIMITERS);
    }

    printf("Correo: %d\n", count_words_correo);
    printf("Articulo: %d\n", count_words_articulo);
    printf("Reporte: %d\n", count_words_reporte);

    /* esto se borraria
        size_t longitud = strlen(documento);

        char *copia = malloc(longitud + 1);

        if (copia == NULL)
        {
            perror("malloc");
            return;
        }

        strcpy(copia, documento);

        // Tokenizacion
        char *palabra = strtok(copia, " \n\t"); // Separacion por espacio, salto de linea y tabulacion

        while (palabra != NULL)
        {
            printf("%s\n", palabra);

            palabra = strtok(NULL, " \n\t");
        } */

    free(copia);

    printf("==================================================\n");
}

static void convertirMinusculas(char *texto)
{

    for (int i = 0; texto[i] != '\0'; i++)
    {
        texto[i] = tolower((unsigned char)texto[i]);
    }
}

static bool existePalabra(const char *palabra,
                          const Dictionary *diccionario)
{
    for (int i = 0; i < diccionario->cantidad; i++)
    {
        if (strcmp(palabra, diccionario->palabras[i]) == 0)
        {
            return true;
        }
    }

    return false;
}