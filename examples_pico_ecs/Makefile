CC     = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -DNDEBUG -O2 #-g

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CC = clang
endif

DEPS   = ../pico_ecs.h

all: benchmark example macros events lifecycle deferred_events

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

benchmark: benchmark.o $(DEPS)
	$(CC) -o benchmark benchmark.o #-g

example: example.o $(DEPS)
	$(CC) -o example example.o #-g

macros: macros.o $(DEPS)
	$(CC) -o macros macros.o #-g

events: events.o $(DEPS)
	$(CC) -o events events.o #-g

lifecycle: lifecycle.o $(DEPS)
	$(CC) -o lifecycle lifecycle.o #-g

deferred_events: deferred_events.o $(DEPS)
	$(CC) -o deferred_events deferred_events.o #-g

.PHONY: clean

clean:
	rm -f benchmark example macros events lifecycle deferred_events *.o
