#include <stdio.h>

#include "user_profile.h"
#include "config.h"

void inicializarPerfil(UserProfile *perfil)
{
    if (perfil == NULL)
    {
        return;
    }

    perfil->correos = 0;
    perfil->articulos = 0;
    perfil->reportes = 0;
}

void registrarDocumento(UserProfile *perfil, DocumentClass clase)
{
    if (perfil == NULL)
    {
        return;
    }

    switch (clase)
    {
    case DOC_CORREO:
        perfil->correos++;
        break;

    case DOC_ARTICULO:
        perfil->articulos++;

    case DOC_REPORTE:
        perfil->reportes++;
        break;

    default:
        break;
    }
}

void imprimirPerfil(const UserProfile *perfil)
{

    if (perfil == NULL)
    {
        return;
    }

    printf("\n========== PERFIL DEL USUARIO ==========\n");

    printf("Correos: %d\n", perfil->correos);
    printf("Artículos: %d\n", perfil->articulos);
    printf("Reportes: %d\n", perfil->reportes);

    printf("========================================\n");
}

UserType clasificarUsuario(const UserProfile *perfil)
{
    /* Se clasifica al usuario segun la distribucion 
    porcentual de su actividad total frente a umbrales 
    predefinidos (config.h)
    
    personal admin, predominancia alta
    de correos y minima actividad en el resto

    Para el tecnico, es compartida entre correos y reportes,
    pocos articulos

    profesor: compartido entre correos y articulos, con pocos
    reportes

    estudiante: compartido articulos y reportes, pocos
    correos

    no detectado: es un perfil nulo que no encaja con los patrones
    anteriores
    */

    if (perfil == NULL)
    {
        return USER_NO_DETECTADO;
    }

    int total = perfil->correos +
                perfil->articulos +
                perfil->reportes;

    if (total == 0)
    {
        return USER_NO_DETECTADO;
    }

    double pCorreo =
        (double)perfil->correos / total;

    double pArticulo =
        (double)perfil->articulos / total;

    double pReporte =
        (double)perfil->reportes / total;

    /*personal administrativo*/

    if (pCorreo >= UMBRAL_DOMINANTE &&
        pArticulo < UMBRAL_MINIMO &&
        pReporte < UMBRAL_MINIMO)
    {
        return USER_ADMINISTRATIVO;
    }

    /*personal tecnico*/

    if (pCorreo >= UMBRAL_COMPLEMENTARIO &&
        pReporte >= UMBRAL_COMPLEMENTARIO &&
        pArticulo < UMBRAL_MINIMO)
    {
        return USER_TECNICO;
    }

    /*profesor*/

    if (pCorreo >= UMBRAL_COMPLEMENTARIO &&
        pArticulo >= UMBRAL_COMPLEMENTARIO &&
        pReporte < UMBRAL_MINIMO)
    {
        return USER_PROFESOR;
    }

    /*estudiante*/

    if (pArticulo >= UMBRAL_COMPLEMENTARIO &&
        pReporte >= UMBRAL_COMPLEMENTARIO &&
        pCorreo < UMBRAL_MINIMO)
    {
        return USER_ESTUDIANTE;
    }

    return USER_NO_DETECTADO;
}

