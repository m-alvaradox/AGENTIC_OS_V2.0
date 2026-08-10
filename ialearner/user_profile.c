#include <stdio.h>
#include <stdbool.h>

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
        break;

    case DOC_REPORTE:
        perfil->reportes++;
        break;

    default:
        break;
    }
}

UserType determinarTipoUsuario(const UserProfile *perfil)
{
    /*
     * Tabla 2:
     * - Administrativo: correo.
     * - Tecnico: correo y reporte.
     * - Profesor: correo y articulo.
     * - Estudiante: articulo y reporte.
     * Si aparecen las tres clases a la vez, no se puede determinar
     * el tipo de usuario.
     */
    bool tieneCorreo;
    bool tieneArticulo;
    bool tieneReporte;

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

    tieneCorreo = perfil->correos > 0;
    tieneArticulo = perfil->articulos > 0;
    tieneReporte = perfil->reportes > 0;

    if (tieneCorreo && tieneArticulo && tieneReporte)
    {
        return USER_NO_DETECTADO;
    }

    if (tieneCorreo && !tieneArticulo && !tieneReporte)
    {
        return USER_ADMINISTRATIVO;
    }

    if (tieneCorreo && !tieneArticulo && tieneReporte)
    {
        return USER_TECNICO;
    }

    if (tieneCorreo && tieneArticulo && !tieneReporte)
    {
        return USER_PROFESOR;
    }

    if (!tieneCorreo && tieneArticulo && tieneReporte)
    {
        return USER_ESTUDIANTE;
    }

    return USER_NO_DETECTADO;
}

UserType clasificarUsuario(const UserProfile *perfil)
{
    UserType tipo;

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

    printf("=========== ESTADISTICAS ===========\n\n");
    printf("%-11s %-2d (%.2f%%)\n", "Correos:", perfil->correos, pCorreo * 100.0);
    printf("%-11s %-2d (%.2f%%)\n", "Articulos:", perfil->articulos, pArticulo * 100.0);
    printf("%-11s %-2d (%.2f%%)\n", "Reportes:", perfil->reportes, pReporte * 100.0);

    tipo = determinarTipoUsuario(perfil);

    return tipo;
}

void imprimirPerfilUsuario(UserType tipo)
{
    printf("\n=========== PERFIL DEL USUARIO ===========\n");

    switch (tipo)
    {
    case USER_ADMINISTRATIVO:
        printf("Personal Administrativo\n");
        break;

    case USER_TECNICO:
        printf("Personal Técnico\n");
        break;

    case USER_PROFESOR:
        printf("Profesor\n");
        break;

    case USER_ESTUDIANTE:
        printf("Estudiante\n");
        break;

    default:
        printf("Desconocido\n");
    }

    printf("==========================================\n");
}
