#ifndef TEMPLATES_H
#define TEMPLATES_H

#include "game.h"

ecs_id_t make_player(game_t* game, int x, int y);
ecs_id_t make_snake(game_t* game, int x, int y);
ecs_id_t make_goblin(game_t* game, int x, int y);
ecs_id_t make_orc(game_t* game, int x, int y);
ecs_id_t make_chest(game_t* game, int x, int y);
ecs_id_t make_king(game_t* game, int x, int y);

#endif // TEMPLATES_H
