#ifndef CONFIG_H
#define CONFIG_H

#define TOKEN_DELIMITERS " ,.;:!?()[]{}\"'\t\r\n"

/*Diccionarios*/
#define MAX_WORD_LENGTH              64
#define MAX_CLASS_NAME               50
#define INITIAL_DICTIONARY_CAPACITY  10

#define DICTIONARY_DIRECTORY "../diccionarios/"
#define EMAIL_DICTIONARY_FILE      "correo.txt"
#define ARTICLE_DICTIONARY_FILE    "articulo.txt"
#define REPORT_DICTIONARY_FILE     "reporte.txt"

#define EMAIL_CLASS_NAME      "Correo"
#define ARTICLE_CLASS_NAME    "Articulo"
#define REPORT_CLASS_NAME     "Reporte"

/*Network*/
#define SERVER_PORT          5000
#define SERVER_IP            "127.0.0.1"
#define SERVER_BACKLOG       10

/*Window*/
#define WINDOW_WIDTH                 400
#define WINDOW_HEIGHT                200
#define WINDOW_BORDER                1
#define WINDOW_POS_X                 10
#define WINDOW_POS_Y                 10

/*Clasificacion Usuario .. Umbrales*/
#define UMBRAL_DOMINANTE      0.60
#define UMBRAL_COMPLEMENTARIO 0.40
#define UMBRAL_MINIMO         0.20

#endif

