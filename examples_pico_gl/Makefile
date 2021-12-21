CC      = gcc
#CC     = clang
CFLAGS = -g -pg -std=c11 -Wall -Wextra -Wpedantic -I .. -I /usr/include/SDL2/ #-Weverything

DEPS   = ../pico_gl.h

all: quad sg

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

quad: quad.o $(DEPS)
	$(CC) -o quad quad.o -lSDL2 -ldl -lm -pg

sg: sg.o $(DEPS)
	$(CC) -o sg sg.o -lSDL2 -ldl -lm -pg

.PHONY: clean

clean:
	rm quad sg *.o*
