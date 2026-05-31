#include "factories.h"
#include "components.h"
#include "math.h"

ecs_entity_t make_player(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_entity_t entity = ecs_create(ecs);

    ecs_add(ecs, entity, POS_COMP);
    pos_t* pos = ecs_get(ecs, entity, POS_COMP);
    pos->x = x;
    pos->y = y;

    ecs_add(ecs, entity, DRAWABLE_COMP);
    drawable_t* drawable = ecs_get(ecs, entity, DRAWABLE_COMP);
    drawable->symbol = '@';
    drawable->visible = true;

    ecs_add(ecs, entity, STATS_COMP);
    stats_t* stats = ecs_get(ecs, entity, STATS_COMP);
    stats->health  = 10;
    stats->attack  = 5;
    stats->defense = 3;

    ecs_add(ecs, entity, PLAYER_COMP);

    return entity;
}

ecs_entity_t make_snake(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_entity_t entity = ecs_create(ecs);

    ecs_add(ecs, entity, POS_COMP);
    pos_t* pos = ecs_get(ecs, entity, POS_COMP);
    pos->x = x;
    pos->y = y;

    ecs_add(ecs, entity, STATS_COMP);
    stats_t* stats = ecs_get(ecs, entity, STATS_COMP);
    stats->health  = 5;
    stats->attack  = 4;
    stats->defense = 1;

    ecs_add(ecs, entity, DRAWABLE_COMP);
    drawable_t* drawable = ecs_get(ecs, entity, DRAWABLE_COMP);
    drawable->symbol = 'S';
    drawable->visible = true;

    ecs_add(ecs, entity, MONSTER_COMP);
    monster_t* monster = ecs_get(ecs, entity, MONSTER_COMP);
    monster->state = MONSTER_WANDER;
    monster->type = MONSTER_SNAKE;

    return entity;
}

ecs_entity_t make_goblin(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_entity_t entity = ecs_create(ecs);

    ecs_add(ecs, entity, POS_COMP);
    pos_t* pos = ecs_get(ecs, entity, POS_COMP);
    pos->x = x;
    pos->y = y;

    ecs_add(ecs, entity, STATS_COMP);
    stats_t* stats = ecs_get(ecs, entity, STATS_COMP);
    stats->health  = 10;
    stats->attack  = 6;
    stats->defense = 2;

    ecs_add(ecs, entity, DRAWABLE_COMP);
    drawable_t* drawable = ecs_get(ecs, entity, DRAWABLE_COMP);
    drawable->visible = true;
    drawable->symbol = 'G';

    ecs_add(ecs, entity, MONSTER_COMP);
    monster_t* monster = ecs_get(ecs, entity, MONSTER_COMP);
    monster->state = MONSTER_WANDER;
    monster->type = MONSTER_GOBLIN;

    return entity;
}

ecs_entity_t make_orc(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_entity_t entity = ecs_create(ecs);

    ecs_add(ecs, entity, POS_COMP);
    pos_t* pos = ecs_get(ecs, entity, POS_COMP);
    pos->x = x;
    pos->y = y;

    ecs_add(ecs, entity, STATS_COMP);
    stats_t* stats = ecs_get(ecs, entity, STATS_COMP);
    stats->health  = 12;
    stats->attack  = 7;
    stats->defense = 3;

    ecs_add(ecs, entity, DRAWABLE_COMP);
    drawable_t* drawable = ecs_get(ecs, entity, DRAWABLE_COMP);
    drawable->visible = true;
    drawable->symbol = 'O';

    ecs_add(ecs, entity, MONSTER_COMP);
    monster_t* monster = ecs_get(ecs, entity, MONSTER_COMP);
    monster->state = MONSTER_WANDER;
    monster->type = MONSTER_ORC;

    return entity;
}

ecs_entity_t make_king(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_entity_t entity = ecs_create(ecs);

    ecs_add(ecs, entity, POS_COMP);
    pos_t* pos = ecs_get(ecs, entity, POS_COMP);
    pos->x = x;
    pos->y = y;

    ecs_add(ecs, entity, STATS_COMP);
    stats_t* stats = ecs_get(ecs, entity, STATS_COMP);
    stats->health  = 20;
    stats->attack  = 10;
    stats->defense = 5;

    ecs_add(ecs, entity, DRAWABLE_COMP);
    drawable_t* drawable = ecs_get(ecs, entity, DRAWABLE_COMP);
    drawable->visible = true;
    drawable->symbol = 'K';

    ecs_add(ecs, entity, MONSTER_COMP);
    monster_t* monster = ecs_get(ecs, entity, MONSTER_COMP);
    monster->state = MONSTER_WANDER;
    monster->type = MONSTER_ORC;

    return entity;
}

ecs_entity_t make_chest(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_entity_t entity = ecs_create(ecs);

    ecs_add(ecs, entity, POS_COMP);
    pos_t* pos = ecs_get(ecs, entity, POS_COMP);
    pos->x = x;
    pos->y = y;

    ecs_add(ecs, entity, CHEST_COMP);
    chest_comp_t* chest = ecs_get(ecs, entity, CHEST_COMP);
    chest->attack = random_int(0, 5);
    chest->health = random_int(0, 5);
    chest->defense = random_int(0, 5);

    ecs_add(ecs, entity, DRAWABLE_COMP);
    drawable_t* drawable = ecs_get(ecs, entity, DRAWABLE_COMP);
    drawable->symbol = 'C';
    drawable->visible = false;

    return entity;
}
