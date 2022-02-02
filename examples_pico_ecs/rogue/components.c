#include "game.h"
#include "components.h"

#include <stdlib.h>
#include <string.h>

//
// Load all components
//
void register_components(game_t* game)
{
    ecs_register_component(game->ecs, COMP_POS,      sizeof(pos_t),        NULL, NULL);
    ecs_register_component(game->ecs, COMP_DRAWABLE, sizeof(drawable_t),   NULL, NULL);
    ecs_register_component(game->ecs, COMP_STATS,    sizeof(stats_t),      NULL, NULL);
    ecs_register_component(game->ecs, COMP_PLAYER,   sizeof(player_t),     NULL, NULL);
    ecs_register_component(game->ecs, COMP_MONSTER,  sizeof(monster_t),    NULL, NULL);
    ecs_register_component(game->ecs, COMP_CHEST,    sizeof(chest_comp_t), NULL, NULL);
}

