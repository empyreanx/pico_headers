CC     = gcc
CFLAGS = -std=c99 -g -Wall -Wextra -Wpedantic

#CC     = clang
#CFLAGS = -g -std=c99 -Wall -Wextra -Wpedantic #-Weverything

SRCS   = main.c

#DEPS   = ../pico_ecs.h
OBJS   = $(SRCS:.c=.o)

all: tests

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

tests: $(OBJS)
	$(CC) -o tests $(OBJS) -lm

.PHONY: clean

clean:
	rm tests *.o
