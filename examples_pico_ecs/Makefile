#CC     = gcc
#CFLAGS = -std=c99 -Wall -pedantic -I ..

CC     = clang
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic -I .. #-Weverything

#CC = g++
#CFLAGS = -g -std=c++11 -Wall -Wextra -Wpedantic -O2 -I .. #-Weverything

DEPS   = ../pico_ecs.h

all: benchmark example

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

benchmark: benchmark.o $(DEPS)
	$(CC) -o benchmark benchmark.o

example: example.o $(DEPS)
	$(CC) -o example example.o

.PHONY: clean

clean:
	rm benchmark example *.o
