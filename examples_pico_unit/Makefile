CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic

#CC     = clang
#CFLAGS = -std=c99 -Wall -Wextra -Wpedantic #-Weverything

DEPS   = ../pico_unit.h

all: example1 example2

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

example1: example1.o $(DEPS)
	$(CC) -o example1 example1.o

example2: example2.o $(DEPS)
	$(CC) -o example2 example2.o

.PHONY: clean

clean:
	rm example1 example2 *.o
