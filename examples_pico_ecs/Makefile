CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CC = clang
endif

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
