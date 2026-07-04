#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classifier.h"

void clasificarDocumento(const char *documento)
{
    printf("\n========== CLASIFICACIÓN DEL DOCUMENTO ==========\n");

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
    }
    
    free(copia);

    printf("==================================================\n");


}