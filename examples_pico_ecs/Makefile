CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -DNDEBUG -O2 #-g

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CC = clang
endif

DEPS   = ../pico_ecs.h

all: benchmark example events messaging macros

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

benchmark: benchmark.o $(DEPS)
	$(CC) -o benchmark benchmark.o #-g

example: example.o $(DEPS)
	$(CC) -o example example.o #-g

events: events.o $(DEPS) ../pico_emitter.h
	$(CC) -o events events.o #-g

messaging: messaging.o $(DEPS) ../pico_emitter.h
	$(CC) -o messaging messaging.o #-g

macros: macros.o $(DEPS)
	$(CC) -o macros macros.o #-g

.PHONY: clean

clean:
	rm -f benchmark example events messaging macros *.o
