#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "pico_ecs.h"

struct game_s;

enum
{
    SYS_PLAYER,
    SYS_MONSTER,
    SYS_CHEST,
    SYS_DRAWABLE
};

typedef enum
{
    MOVE_LEFT,
    MOVE_RIGHT,
    MOVE_UP,
    MOVE_DOWN,
    MOVE_NONE
} move_t;

void register_systems(struct game_s* game);

#endif // SYSTEMS_H
