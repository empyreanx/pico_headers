ifeq ($(OS),Windows_NT)
    CC     = gcc
    CFLAGS = -g -std=c11 -Wall -Wextra -Wpedantic -I .
    CLIBS  = -lmingw32 -lwinmm -lSDL2main -lSDL2 -lm
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        CC     = gcc
        CFLAGS = -g -std=c11 -Wall -Wextra -Wpedantic -I .
        CLIBS  = -lSDL2 -ldl -lm -lGL
    endif
    ifeq ($(UNAME_S),Darwin)
        CC     = clang
        CFLAGS = -g -std=c11 -Wall -Wextra -Wpedantic -I .
        CLIBS  = -lSDL2 -ldl -lm
    endif
endif

DEPS   = ../pico_gl.h

#all: quad sg

all: quad

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

quad: quad.o $(DEPS)
	$(CC) -o quad quad.o $(CLIBS)

#sg: sg.o $(DEPS)
#	$(CC) -o sg sg.o $(CLIBS)

.PHONY: clean

clean:
	rm -f quad sg *.o*
