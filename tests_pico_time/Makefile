CC     = gcc
CFLAGS = -std=c99 -g -Wall -Wextra -Wpedantic #-D_POSIX_C_SOURCE=199309L

ifeq ($(OS),Windows_NT)
    LIBS = -lmingw32 -lwinmm
else
    LIBS =
endif

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
    CC = clang
endif

SRCS   = main.c

DEPS   = ../pico_time.h
OBJS   = $(SRCS:.c=.o)

all: tests

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

tests: $(OBJS)
	$(CC) -o tests $(OBJS) $(LIBS)

.PHONY: clean

clean:
	rm tests *.o
