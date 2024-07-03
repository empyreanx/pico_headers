CFLAGS = $(shell pkg-config --cflags sdl2)
CLIBS  = $(shell pkg-config --libs sdl2)

ifeq ($(OS),Windows_NT)
    CC     = clang # gcc produces too many spurious warnings
    CFLAGS += -g -std=c11 -Wall -Wextra -Wpedantic -I .
    CLIBS  += -lmingw32 -lwinmm -lSDL2main -lSDL2 -lm
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CC     = gcc
        CFLAGS += -g -std=c11 -Wall -Wextra -Wpedantic -I . #-pg
        CLIBS  += -lSDL2 -ldl -lm -lGL# -pg
    endif
    ifeq ($(UNAME_S),Darwin)
        CC     = clang
        CFLAGS += -g -std=c11 -Wall -Wextra -Wpedantic -I .
        CLIBS  += -lSDL2 -ldl -lm -framework OpenGL
    endif
endif

DEPS   = ../pico_gfx.h

all: quad scenegraph particles

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

quad: quad.o $(DEPS)
	$(CC) -o quad quad.o $(CLIBS)

scenegraph: scenegraph.o $(DEPS)
	$(CC) -o scenegraph scenegraph.o $(CLIBS)

particles: particles.o $(DEPS)
	$(CC) -o particles particles.o $(CLIBS)

.PHONY: clean

clean:
	rm -f quad scenegraph particles *.o
