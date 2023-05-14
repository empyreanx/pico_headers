#include "game.h"
#include "components.h"

#include <stdlib.h>
#include <string.h>

ecs_id_t POS_COMP;
ecs_id_t DRAWABLE_COMP;
ecs_id_t STATS_COMP;
ecs_id_t PLAYER_COMP;
ecs_id_t MONSTER_COMP;
ecs_id_t CHEST_COMP;

//
// Load all components
//
void register_components(game_t* game)
{
    POS_COMP      = ecs_register_component(game->ecs, sizeof(pos_t), NULL, NULL);
    DRAWABLE_COMP = ecs_register_component(game->ecs, sizeof(drawable_t), NULL, NULL);
    STATS_COMP    = ecs_register_component(game->ecs, sizeof(stats_t), NULL, NULL);
    PLAYER_COMP   = ecs_register_component(game->ecs, sizeof(player_t), NULL, NULL);
    MONSTER_COMP  = ecs_register_component(game->ecs, sizeof(monster_t), NULL, NULL);
    CHEST_COMP    = ecs_register_component(game->ecs, sizeof(chest_comp_t), NULL, NULL);
}

