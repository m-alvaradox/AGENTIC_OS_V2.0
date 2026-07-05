# Makefile - Launcher

CC=gcc

CFLAGS = -Wall -Wextra -g -I../include

TARGET = launcher

SRCS = launcher.c launcher_ui.c process_manager.c launcher_protocol.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

rebuild: clean all