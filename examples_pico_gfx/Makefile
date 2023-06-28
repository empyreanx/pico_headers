ifeq ($(OS),Windows_NT)
    CC     = gcc
    CFLAGS = -g -std=c11 -Wall -Wextra -Wpedantic -I .
    CLIBS  = -lmingw32 -lwinmm -lSDL2main -lSDL2 -lGL -lm
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CC     = gcc
        CFLAGS = -g -std=c11 -Wall -Wextra -Wpedantic -I . -pg
        CLIBS  = -lSDL2 -ldl -lm -lGL -pg
    endif
    ifeq ($(UNAME_S),Darwin)
        CC     = clang
        CFLAGS = -g -std=c11 -Wall -Wextra -Wpedantic -I .
        CLIBS  = -lSDL2 -ldl -lm
    endif
endif

DEPS   = ../pico_gfx.h

all: quad scenegraph

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

quad: quad.o $(DEPS)
	$(CC) -o quad quad.o $(CLIBS)

scenegraph: scenegraph.o $(DEPS)
	$(CC) -o scenegraph scenegraph.o $(CLIBS)

.PHONY: clean

clean:
	rm -f quad scenegraph *.o
