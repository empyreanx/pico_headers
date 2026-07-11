CC     = gcc
CFLAGS = -std=c11 -g -Wall -Wextra -Wpedantic

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    CC = clang
endif

SRCS   = main.c entity.c systems.c deferred.c set.c hierarchy.c

DEPS   = ../pico_ecs.h common.h
OBJS   = $(SRCS:.c=.o)

all: tests

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

tests: $(OBJS)
	$(CC) -o tests $(OBJS) -lm

.PHONY: clean

clean:
	rm -f tests *.o
