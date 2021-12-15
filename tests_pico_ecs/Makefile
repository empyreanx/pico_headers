CC     = gcc
CFLAGS = -std=c99 -g -Wall -pedantic -I ..

#CC     = clang
#CFLAGS = -g -std=c99 -Wall -Wextra -Wpedantic -I .. #-Weverything

SRCS   = main.c

#DEPS   = ../pico_ht.h
OBJS   = $(SRCS:.c=.o)

all: tests

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

tests: $(OBJS)
	$(CC) -o tests $(OBJS) -lm

.PHONY: clean

clean:
	rm tests *.o
