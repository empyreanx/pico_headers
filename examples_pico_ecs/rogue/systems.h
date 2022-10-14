#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "pico_ecs.h"

struct game_s;

extern ecs_id_t PLAYER_SYS;
extern ecs_id_t MONSTER_SYS;
extern ecs_id_t CHEST_SYS;
extern ecs_id_t DRAWABLE_SYS;

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
