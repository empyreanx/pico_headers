CC     = gcc
CFLAGS = -std=c99 -g -Wall -Wextra -Wpedantic #-DNDEBUG

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CC = clang
endif

SRCS   = main.c

DEPS   = ../pico_qt.h
OBJS   = $(SRCS:.c=.o)

all: tests

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

tests: $(OBJS)
	$(CC) -o tests $(OBJS) -lm

.PHONY: clean

clean:
	rm -f tests *.o
