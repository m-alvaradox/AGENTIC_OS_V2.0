#ifndef DICTIONARY_H
#define DICTIONARY_H

typedef struct
{
    char nombre[50];
    char **palabras;
    int cantidad;
    int capacidad;
} Dictionary;

Dictionary *cargarDiccionario(const char *nombreArchivo,
                              const char *nombreClase);

void liberarDiccionario(Dictionary *diccionario);

#endif