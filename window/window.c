#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <sys/socket.h>

#include "network.h"

int main(void)
{
    Display *display = XOpenDisplay(NULL);
    if (!display)
    {
        fprintf(stderr, "Cannot open display\n");
        return 1;
    }

    // Connect to the server
    int socket_fd = conectarServidor();

    if (socket_fd == -1)
    {
        return 1;
    }

    int screen = DefaultScreen(display);

    Window window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        10, 10, 400, 200,
        1,
        BlackPixel(display, screen),
        WhitePixel(display, screen));

    XSelectInput(display, window, ExposureMask | KeyPressMask);
    XMapWindow(display, window);

    XEvent event;

    while (1)
    {
        XNextEvent(display, &event);

        if (event.type == KeyPress)
        {

            KeySym keysym;  // detect escape, enter, other special keys
            char buffer[2]; // buffer to hold the character
            int caracteres; // number of characters read

            caracteres = XLookupString(&event.xkey,
                                       buffer,
                                       sizeof(buffer),
                                       &keysym,
                                       NULL);
            // XLookupString representa que caracter produjo esa tecla considerando shift, capslock, etc.
            // si usuario pulsa Espacio, buffer[0] = ' ', si pulsa Enter, buffer[0] = '\n', etc.
            
            if (caracteres > 0) {
                // debug
                printf("Caracter presionado: %c\n", buffer[0]);

                int enviados = send(socket_fd, &buffer[0], 1, 0);

                if (enviados == -1)
                {
                    perror("Error enviando caracter al servidor");
                }
            }

            if (keysym == XK_Escape)
                break; 
        }
    }

    // Close connection to server
    cerrarConexion(socket_fd);

    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
