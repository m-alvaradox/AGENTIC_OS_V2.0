#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "dictionary.h"
#include "utils.h"

struct DictionaryNode
{
    char *palabra;
    struct DictionaryNode *siguiente;
};

static unsigned long hashPalabra(const char *palabra)
{
    unsigned long hash = 5381;
    int c;

    while ((c = (unsigned char)*palabra++) != '\0')
    {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

static char *copiarCadena(const char *cadena)
{
    char *copia = malloc(strlen(cadena) + 1);

    if (copia == NULL)
    {
        return NULL;
    }

    strcpy(copia, cadena);
    return copia;
}

static int insertarPalabra(Dictionary *diccionario,
                           const char *palabra)
{
    if (diccionario == NULL ||
        palabra == NULL ||
        diccionario->capacidad == 0)
    {
        return -1;
    }

    unsigned long indice =
        hashPalabra(palabra) %
        (unsigned long)diccionario->capacidad;

    DictionaryNode *actual = diccionario->tabla[indice];

    while (actual != NULL)
    {
        if (strcmp(actual->palabra, palabra) == 0)
        {
            return 0;
        }

        actual = actual->siguiente;
    }

    DictionaryNode *nuevo = malloc(sizeof(DictionaryNode));

    if (nuevo == NULL)
    {
        return -1;
    }

    nuevo->palabra = copiarCadena(palabra);

    if (nuevo->palabra == NULL)
    {
        free(nuevo);
        return -1;
    }

    nuevo->siguiente = diccionario->tabla[indice];
    diccionario->tabla[indice] = nuevo;
    diccionario->cantidad++;

    return 0;
}

static int aumentarCapacidad(Dictionary *diccionario)
{
    int nuevaCapacidad = diccionario->capacidad * 2;

    DictionaryNode **nuevaTabla = calloc(nuevaCapacidad,
                                         sizeof(DictionaryNode *));

    if (nuevaTabla == NULL)
    {
        return -1;
    }

    for (int i = 0; i < diccionario->capacidad; i++)
    {
        DictionaryNode *actual = diccionario->tabla[i];

        while (actual != NULL)
        {
            DictionaryNode *siguiente = actual->siguiente;
            unsigned long indice =
                hashPalabra(actual->palabra) %
                (unsigned long)nuevaCapacidad;

            actual->siguiente = nuevaTabla[indice];
            nuevaTabla[indice] = actual;
            actual = siguiente;
        }
    }

    free(diccionario->tabla);
    diccionario->tabla = nuevaTabla;
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


    diccionario->tabla = calloc(diccionario->capacidad,
                                sizeof(DictionaryNode *));

    if (diccionario->tabla == NULL)
    {
        fclose(archivo);
        free(diccionario);
        return NULL;
    }

    char buffer[MAX_WORD_LENGTH];

    // read each word of dict
    while (fgets(buffer, MAX_WORD_LENGTH, archivo) != NULL)
    {
        // strip "\n"
        buffer[strcspn(buffer, "\n")] = '\0';
        convertirMinusculas(buffer);

        if (buffer[0] == '\0')
        {
            continue;
        }

        if (diccionario->cantidad * 4 >= diccionario->capacidad * 3 &&
            aumentarCapacidad(diccionario) == -1)
        {
            liberarDiccionario(diccionario);
            fclose(archivo);
            return NULL;
        }

        if (insertarPalabra(diccionario, buffer) == -1)
        {
            liberarDiccionario(diccionario);
            fclose(archivo);
            return NULL;
        }
    }
    fclose(archivo);

    return diccionario;
}

void liberarDiccionario(Dictionary *diccionario)
{

    if (diccionario == NULL)
    {
        return;
    }

    for (int i = 0; i < diccionario->capacidad; i++)
    {
        DictionaryNode *actual = diccionario->tabla[i];

        while (actual != NULL)
        {
            DictionaryNode *siguiente = actual->siguiente;

            free(actual->palabra);
            free(actual);

            actual = siguiente;
        }
    }

    free(diccionario->tabla);
    free(diccionario);

}

bool diccionarioContiene(const Dictionary *diccionario,
                         const char *palabra)
{
    if (diccionario == NULL ||
        palabra == NULL ||
        diccionario->capacidad == 0)
    {
        return false;
    }

    unsigned long indice =
        hashPalabra(palabra) %
        (unsigned long)diccionario->capacidad;

    DictionaryNode *actual = diccionario->tabla[indice];

    while (actual != NULL)
    {
        if (strcmp(actual->palabra, palabra) == 0)
        {
            return true;
        }

        actual = actual->siguiente;
    }

    return false;
}
