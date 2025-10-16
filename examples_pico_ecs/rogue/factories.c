#include "factories.h"
#include "components.h"
#include "math.h"

ecs_entity_t make_player(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_entity_t entity = ecs_create(ecs);

    pos_t* pos = ecs_add(ecs, entity, POS_COMP, NULL);
    pos->x = x;
    pos->y = y;

    drawable_t* drawable = ecs_add(ecs, entity, DRAWABLE_COMP, NULL);
    drawable->symbol = '@';
    drawable->visible = true;

    stats_t* stats = ecs_add(ecs, entity, STATS_COMP, NULL);
    stats->health  = 10;
    stats->attack  = 5;
    stats->defense = 3;

    ecs_add(ecs, entity, PLAYER_COMP, NULL);

    return entity;
}

ecs_entity_t make_snake(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_entity_t entity = ecs_create(ecs);

    pos_t* pos = ecs_add(ecs, entity, POS_COMP, NULL);
    pos->x = x;
    pos->y = y;

    stats_t* stats = ecs_add(ecs, entity, STATS_COMP, NULL);
    stats->health  = 5;
    stats->attack  = 4;
    stats->defense = 1;

    drawable_t* drawable = ecs_add(ecs, entity, DRAWABLE_COMP, NULL);
    drawable->symbol = 'S';
    drawable->visible = true;

    monster_t* monster = ecs_add(ecs, entity, MONSTER_COMP, NULL);
    monster->state = MONSTER_WANDER;
    monster->type = MONSTER_SNAKE;

    return entity;
}

ecs_entity_t make_goblin(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_entity_t entity = ecs_create(ecs);

    pos_t* pos = ecs_add(ecs, entity, POS_COMP, NULL);
    pos->x = x;
    pos->y = y;

    stats_t* stats = ecs_add(ecs, entity, STATS_COMP, NULL);
    stats->health  = 10;
    stats->attack  = 6;
    stats->defense = 2;

    drawable_t* drawable = ecs_add(ecs, entity, DRAWABLE_COMP, NULL);
    drawable->visible = true;
    drawable->symbol = 'G';

    monster_t* monster = ecs_add(ecs, entity, MONSTER_COMP, NULL);
    monster->state = MONSTER_WANDER;
    monster->type = MONSTER_GOBLIN;

    return entity;
}

ecs_entity_t make_orc(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_entity_t entity = ecs_create(ecs);

    pos_t* pos = ecs_add(ecs, entity, POS_COMP, NULL);
    pos->x = x;
    pos->y = y;

    stats_t* stats = ecs_add(ecs, entity, STATS_COMP, NULL);
    stats->health  = 12;
    stats->attack  = 7;
    stats->defense = 3;

    drawable_t* drawable = ecs_add(ecs, entity, DRAWABLE_COMP, NULL);
    drawable->visible = true;
    drawable->symbol = 'O';

    monster_t* monster = ecs_add(ecs, entity, MONSTER_COMP, NULL);
    monster->state = MONSTER_WANDER;
    monster->type = MONSTER_ORC;

    return entity;
}

ecs_entity_t make_king(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_entity_t entity = ecs_create(ecs);

    pos_t* pos = ecs_add(ecs, entity, POS_COMP, NULL);
    pos->x = x;
    pos->y = y;

    stats_t* stats = ecs_add(ecs, entity, STATS_COMP, NULL);
    stats->health  = 20;
    stats->attack  = 10;
    stats->defense = 5;

    drawable_t* drawable = ecs_add(ecs, entity, DRAWABLE_COMP, NULL);
    drawable->visible = true;
    drawable->symbol = 'K';

    monster_t* monster = ecs_add(ecs, entity, MONSTER_COMP, NULL);
    monster->state = MONSTER_WANDER;
    monster->type = MONSTER_ORC;

    return entity;
}

ecs_entity_t make_chest(game_t* game, int x, int y)
{
    ecs_t* ecs = game->ecs;
    ecs_entity_t entity = ecs_create(ecs);

    pos_t* pos = ecs_add(ecs, entity, POS_COMP, NULL);
    pos->x = x;
    pos->y = y;

    chest_comp_t* chest = ecs_add(ecs, entity, CHEST_COMP, NULL);
    chest->attack = random_int(0, 5);
    chest->health = random_int(0, 5);
    chest->defense = random_int(0, 5);

    drawable_t* drawable = ecs_add(ecs, entity, DRAWABLE_COMP, NULL);
    drawable->symbol = 'C';
    drawable->visible = false;

    return entity;
}
