#include <ctype.h>
#include "utils.h"

void convertirMinusculas(char *texto)
{
    for (int i = 0; texto[i] != '\0'; i++)
    {
        texto[i] = tolower((unsigned char)texto[i]);
    }
}