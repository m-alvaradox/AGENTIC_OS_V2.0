#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/socket.h>

#include "network.h"
#include "config.h"

int main(int argc, char **argv)
{
    int puerto = DOCUMENT_SERVICE_PORT;

    signal(SIGPIPE, SIG_IGN);

    if (argc > 1)
    {
        puerto = atoi(argv[1]);

        if (puerto <= 0)
        {
            puerto = DOCUMENT_SERVICE_PORT;
        }
    }

    Display *display = XOpenDisplay(NULL);
    if (!display)
    {
        fprintf(stderr, "Cannot open display\n");
        return 1;
    }

    int socket_fd = conectarServidor(puerto);

    if (socket_fd == -1)
    {
        XCloseDisplay(display);
        return 1;
    }

    int screen = DefaultScreen(display);

    Window window = XCreateSimpleWindow(
        display,
        RootWindow(display, screen),
        WINDOW_POS_X, WINDOW_POS_Y, WINDOW_WIDTH, WINDOW_HEIGHT,
        WINDOW_BORDER,
        BlackPixel(display, screen),
        WhitePixel(display, screen));

    Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wmDeleteMessage, 1);

    XSelectInput(display, window, ExposureMask | KeyPressMask | StructureNotifyMask);
    XMapWindow(display, window);

    XEvent event;

    while (1)
    {
        XNextEvent(display, &event);

        if (event.type == ClientMessage &&
            event.xclient.data.l[0] == (long)wmDeleteMessage)
        {
            break;
        }

        if (event.type == KeyPress)
        {
            KeySym keysym;
            char buffer[2];
            int caracteres;

            caracteres = XLookupString(&event.xkey,
                                       buffer,
                                       sizeof(buffer),
                                       &keysym,
                                       NULL);

            if (keysym == XK_Escape)
            {
                break;
            }

            if (keysym == XK_Return)
            {
                if (enviarCaracter(socket_fd, '\n') == -1)
                {
                    break;
                }

                continue;
            }

            if (caracteres > 0)
            {
                if (enviarCaracter(socket_fd, buffer[0]) == -1)
                {
                    break;
                }
            }
        }
    }

    cerrarConexion(socket_fd);

    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return 0;
}
