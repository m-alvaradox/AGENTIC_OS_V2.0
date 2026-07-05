#ifndef PROTOCOL_H
#define PROTOCOL_H

typedef enum
{
    CMD_START = 1,
    CMD_END

} ControlCommand;

typedef enum
{
    USER_NO_DETECTADO = 0,
    USER_ADMINISTRATIVO,
    USER_TECNICO,
    USER_PROFESOR,
    USER_ESTUDIANTE

} UserType;

typedef struct
{
    UserType tipo;

} UserContext;

#endif