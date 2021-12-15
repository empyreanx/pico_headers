#CC     = clang
CC      = gcc
MAKE   = make
CFLAGS = -g -std=c99 -Wall -Wextra -Wpedantic -I ../.. #-Weverything
DEPS   = ../../pico_ecs.h
SRCS   = game.c       \
         components.c \
         levels.c     \
         systems.c    \
         factories.c  \
         tilemaps.c   \
         math.c       \
         files.c      \
         hud.c

OBJS   = $(SRCS:.c=.o)

all: $(OBJS) rogue

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

rogue: $(OBJS)
	$(CC) -o rogue $(OBJS) -lncurses

.PHONY: clean

clean:
	rm rogue *.o

