#include "factories.h"
#include "components.h"
#include "math.h"

ecs_id_t make_player(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_id_t id = ecs_create(ecs);

    pos_t* pos = ecs_add(ecs, id, POS_COMP, NULL);
    pos->x = x;
    pos->y = y;

    drawable_t* drawable = ecs_add(ecs, id, DRAWABLE_COMP, NULL);
    drawable->symbol = '@';
    drawable->visible = true;

    stats_t* stats = ecs_add(ecs, id, STATS_COMP, NULL);
    stats->health  = 10;
    stats->attack  = 5;
    stats->defense = 3;

    ecs_add(ecs, id, PLAYER_COMP, NULL);

    return id;
}

ecs_id_t make_snake(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_id_t id = ecs_create(ecs);

    pos_t* pos = ecs_add(ecs, id, POS_COMP, NULL);
    pos->x = x;
    pos->y = y;

    stats_t* stats = ecs_add(ecs, id, STATS_COMP, NULL);
    stats->health  = 5;
    stats->attack  = 4;
    stats->defense = 1;

    drawable_t* drawable = ecs_add(ecs, id, DRAWABLE_COMP, NULL);
    drawable->symbol = 'S';
    drawable->visible = true;

    monster_t* monster = ecs_add(ecs, id, MONSTER_COMP, NULL);
    monster->state = MONSTER_WANDER;
    monster->type = MONSTER_SNAKE;

    return id;
}

ecs_id_t make_goblin(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_id_t id = ecs_create(ecs);

    pos_t* pos = ecs_add(ecs, id, POS_COMP, NULL);
    pos->x = x;
    pos->y = y;

    stats_t* stats = ecs_add(ecs, id, STATS_COMP, NULL);
    stats->health  = 10;
    stats->attack  = 6;
    stats->defense = 2;

    drawable_t* drawable = ecs_add(ecs, id, DRAWABLE_COMP, NULL);
    drawable->visible = true;
    drawable->symbol = 'G';

    monster_t* monster = ecs_add(ecs, id, MONSTER_COMP, NULL);
    monster->state = MONSTER_WANDER;
    monster->type = MONSTER_GOBLIN;

    return id;
}

ecs_id_t make_orc(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_id_t id = ecs_create(ecs);

    pos_t* pos = ecs_add(ecs, id, POS_COMP, NULL);
    pos->x = x;
    pos->y = y;

    stats_t* stats = ecs_add(ecs, id, STATS_COMP, NULL);
    stats->health  = 12;
    stats->attack  = 7;
    stats->defense = 3;

    drawable_t* drawable = ecs_add(ecs, id, DRAWABLE_COMP, NULL);
    drawable->visible = true;
    drawable->symbol = 'O';

    monster_t* monster = ecs_add(ecs, id, MONSTER_COMP, NULL);
    monster->state = MONSTER_WANDER;
    monster->type = MONSTER_ORC;

    return id;
}

ecs_id_t make_king(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_id_t id = ecs_create(ecs);

    pos_t* pos = ecs_add(ecs, id, POS_COMP, NULL);
    pos->x = x;
    pos->y = y;

    stats_t* stats = ecs_add(ecs, id, STATS_COMP, NULL);
    stats->health  = 20;
    stats->attack  = 10;
    stats->defense = 5;

    drawable_t* drawable = ecs_add(ecs, id, DRAWABLE_COMP, NULL);
    drawable->visible = true;
    drawable->symbol = 'K';

    monster_t* monster = ecs_add(ecs, id, MONSTER_COMP, NULL);
    monster->state = MONSTER_WANDER;
    monster->type = MONSTER_ORC;

    return id;
}

ecs_id_t make_chest(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_id_t id = ecs_create(ecs);

    pos_t* pos = ecs_add(ecs, id, POS_COMP, NULL);
    pos->x = x;
    pos->y = y;

    chest_comp_t* chest = ecs_add(ecs, id, CHEST_COMP, NULL);
    chest->attack = random_int(0, 5);
    chest->health = random_int(0, 5);
    chest->defense = random_int(0, 5);

    drawable_t* drawable = ecs_add(ecs, id, DRAWABLE_COMP, NULL);
    drawable->symbol = 'C';
    drawable->visible = false;

    return id;
}
