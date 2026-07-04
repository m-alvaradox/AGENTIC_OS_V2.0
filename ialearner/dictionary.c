#include <stdio.h>
#include <stdlib.h>

#include "config.h";
#include "dictionary.h";

Dictionary *cargarDiccionario(const char *nombreArchivo,
                              const char *nombreClase)
{
    FILE *archivo = fopen(nombreArchivo, "r");

    if (archivo == NULL)
    {
        return NULL;
    }

    Dictionary *diccionario = malloc(sizeof(Dictionary));

    if (diccionario == NULL)
    {
        fclose(archivo); // free resources
        return NULL;
    }

    strncpy(diccionario->nombre, nombreClase, MAX_CLASS_NAME -1);
    diccionario->nombre[MAX_CLASS_NAME -1] = '\0';

    diccionario->cantidad = 0;
    diccionario->capacidad = INITIAL_DICTIONARY_CAPACITY;

    diccionario->palabras = malloc(diccionario->capacidad * sizeof(char *));

    if (diccionario->palabras == NULL) {
        fclose(archivo);
        free(diccionario);
        return NULL;
    }

}