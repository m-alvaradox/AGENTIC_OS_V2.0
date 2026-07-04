CC=gcc

CFLAGS=-Wall -Wextra -I../include

OBJ=launcher.o \
    launcher_ui.o \
    process_manager.o \
    launcher_protocol.o

launcher: $(OBJ)
	$(CC) $(OBJ) -o launcher

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o launcher