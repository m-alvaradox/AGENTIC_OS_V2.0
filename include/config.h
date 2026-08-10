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
#define DOCUMENT_SERVICE_PORT          5000
#define CONTROL_SERVICE_PORT           5001
#define WINDOW_BASE_PORT               5100
#define MAX_WINDOW_SERVERS             16
#define MAX_SESSIONS                   16
#define SERVER_IP                     "127.0.0.1"
#define SERVER_BACKLOG                10

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
#define SHOW_DOCUMENT_DEBUG   1

/*Deteccion concurrente*/
#define DEFAULT_DETECTION_THREADS       2
#define INITIAL_SENTENCE_QUEUE_CAPACITY 16

/*Launcher*/
//Process Manager
#define WINDOW_EXECUTABLE "../window/window"
#define INITIAL_CAPACITY 10

//Threads Manager
#define INITIAL_THREAD_CAPACITY 10

#endif
