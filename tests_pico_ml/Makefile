CC     = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wpedantic

#CC     = clang
#CFLAGS = -g -std=c99 -Wall -Wextra -Wpedantic #-Weverything

SRCS   = main.c scalar.c b2.c t2.c v2.c rng.c

DEPS   = ../pico-ml.h
OBJS   = $(SRCS:.c=.o)

all: tests

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

tests: $(OBJS)
	$(CC) -o tests $(OBJS) -lm

.PHONY: clean

clean:
	rm tests *.o
