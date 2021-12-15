#ifndef COMPONENTS_H
#define COMPONENTS_H

#include <stdbool.h>
#include <stdint.h>

struct game_s;

enum
{
    COMP_POS,
    COMP_DRAWABLE,
    COMP_STATS,
    COMP_PLAYER,
    COMP_MONSTER,
    COMP_CHEST
};

typedef struct
{
    int cmd;
} player_t;

typedef struct
{
    char symbol;
    bool visible;
} drawable_t;

typedef struct
{
    int health;
    int attack;
    int defense;
} stats_t;

typedef struct
{
    int health;
    int attack;
    int defense;
} chest_comp_t;

typedef enum
{
    MONSTER_SNAKE,
    MONSTER_GOBLIN,
    MONSTER_ORC,
    MONSTER_KING
} monster_type_t;

typedef enum
{
    MONSTER_WANDER,
    MONSTER_PURSUE,
    MONSTER_ATTACK,
    MONSTER_FLEE
} monster_state_t;

typedef struct
{
    monster_type_t  type;
    monster_state_t state;
} monster_t;

void register_components(struct game_s* game);

#endif // COMPONENTS_H
