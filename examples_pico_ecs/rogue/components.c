#include "game.h"
#include "components.h"

#include <stdlib.h>
#include <string.h>

ecs_id_t g_pos_comp_id;
ecs_id_t g_drawable_comp_id;
ecs_id_t g_stats_comp_id;
ecs_id_t g_player_comp_id;
ecs_id_t g_monster_comp_id;
ecs_id_t g_chest_comp_id;

//
// Load all components
//
void register_components(game_t* game)
{
    g_pos_comp_id      = ecs_register_component(game->ecs, sizeof(pos_t));
    g_drawable_comp_id = ecs_register_component(game->ecs, sizeof(drawable_t));
    g_stats_comp_id    = ecs_register_component(game->ecs, sizeof(stats_t));
    g_player_comp_id   = ecs_register_component(game->ecs, sizeof(player_t));
    g_monster_comp_id  = ecs_register_component(game->ecs, sizeof(monster_t));
    g_chest_comp_id    = ecs_register_component(game->ecs, sizeof(chest_comp_t));
}

