#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <stdbool.h>

typedef struct DictionaryNode DictionaryNode;

typedef struct
{
    char nombre[50];
    DictionaryNode **tabla;
    int cantidad;
    int capacidad;
} Dictionary;

Dictionary *cargarDiccionario(const char *nombreArchivo,
                              const char *nombreClase);

void liberarDiccionario(Dictionary *diccionario);

bool diccionarioContiene(const Dictionary *diccionario,
                         const char *palabra);

#endif
