# Makefile - IA Learner

CC = gcc

CFLAGS = -Wall -Wextra -g -MMD -MP -I../include -I../utils
LDFLAGS = -pthread

TARGET = ialearner

SRCS = ialearner.c server.c client_handler.c classifier.c dictionary.c user_profile.c control_server.c thread_manager.c session_context.c sentence_queue.c ../utils/utils.c
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(DEPS) $(TARGET)

rebuild: clean all

-include $(DEPS)
