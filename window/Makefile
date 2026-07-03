# Makefile - Window

CC = gcc

CFLAGS = -Wall -Wextra -g
LDFLAGS = -lX11

TARGET = window

SRCS = window.c network.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

rebuild: clean all