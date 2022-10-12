#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "pico_ecs.h"

struct game_s;

extern ecs_id_t g_player_sys_id;
extern ecs_id_t g_monster_sys_id;
extern ecs_id_t g_chest_sys_id;
extern ecs_id_t g_drawable_sys_id;

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
