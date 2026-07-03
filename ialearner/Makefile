# Makefile - IA Learner

CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -pthread
TARGET = ialearner
SRCS = ialearner.c server.c client_handler.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET)

rebuild: clean all