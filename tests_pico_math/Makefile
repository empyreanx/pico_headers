CC     = gcc
CFLAGS = -g -std=c99 -Wall -Wextra -Wpedantic

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CC = clang
endif

SRCS   = main.c scalar.c b2.c t2.c v2.c

DEPS   = ../pico_math.h
OBJS   = $(SRCS:.c=.o)

all: tests

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

tests: $(OBJS)
	$(CC) -o tests $(OBJS) -lm

.PHONY: clean

clean:
	rm -f tests *.o
