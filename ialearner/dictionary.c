#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "dictionary.h"
#include "utils.h"

// private function
static int aumentarCapacidad(Dictionary *diccionario)
{

    int nuevaCapacidad = diccionario->capacidad * 2;

    char **temp = realloc(diccionario->palabras, nuevaCapacidad * sizeof(char *));

    if (temp == NULL)
    {
        return -1;
    }

    diccionario->palabras = temp;

    // clean old to new capacity, init
    for (int i = diccionario->capacidad; i < nuevaCapacidad; i++)
    {
        diccionario->palabras[i] = NULL;
    }

    diccionario->capacidad = nuevaCapacidad;

    return 0;
}

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

    strncpy(diccionario->nombre, nombreClase, MAX_CLASS_NAME - 1);
    diccionario->nombre[MAX_CLASS_NAME - 1] = '\0';

    diccionario->cantidad = 0;
    diccionario->capacidad = INITIAL_DICTIONARY_CAPACITY;


    diccionario->palabras = malloc(diccionario->capacidad * sizeof(char *));

    if (diccionario->palabras == NULL)
    {
        fclose(archivo);
        free(diccionario);
        return NULL;
    }

    // Init punteros
    for (int i = 0; i < diccionario->capacidad; i++)
    {
        diccionario->palabras[i] = NULL;
    }

    char buffer[MAX_WORD_LENGTH];

    // read each word of dict
    while (fgets(buffer, MAX_WORD_LENGTH, archivo) != NULL)
    {
        // strip "\n"
        buffer[strcspn(buffer, "\n")] = '\0';
        convertirMinusculas(buffer);
        

        if (diccionario->cantidad == diccionario->capacidad)
        {
            if (aumentarCapacidad(diccionario) == -1)
            {
                liberarDiccionario(diccionario);
                fclose(archivo);
                return NULL;
            }
        }

        char *palabra = malloc(strlen(buffer) + 1);

        if (palabra == NULL)
        {
            liberarDiccionario(diccionario);
            fclose(archivo);
            return NULL;
        }

        strcpy(palabra, buffer);

        diccionario->palabras[diccionario->cantidad] = palabra;

        diccionario->cantidad++;
    }
    fclose(archivo);

    return diccionario;
}

void liberarDiccionario(Dictionary *diccionario) {

    if (diccionario == NULL) {
        return;
    }

    for (int i = 0; i < diccionario->cantidad; i++){
        free(diccionario->palabras[i]);
    }

    free(diccionario->palabras);
    free(diccionario);

}