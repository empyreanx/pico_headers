#ifndef GAME_H
#define GAME_H

#include "components.h"
#include "systems.h"
#include "tilemaps.h"

#include "pico_ecs.h"

enum
{
    LEVEL_OVER = 1,
    GAME_OVER
};

typedef struct game_s
{
    bool player_dead;
    int level;

    ecs_t* ecs;
    ecs_id_t player_id;

    dim_t term_size;

    int level_count;
    level_t** levels;
    tilemap_t** maps;

    tilemap_t* map;

    bool quit;
} game_t;

#endif // GAME_H
