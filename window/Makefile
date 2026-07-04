# Makefile - Window

CC = gcc

CFLAGS = -Wall -Wextra -g -I../network -I../include
LDFLAGS = -lX11

TARGET = window

SRCS = window.c ../network/network.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

rebuild: clean all