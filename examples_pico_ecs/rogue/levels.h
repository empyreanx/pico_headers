#ifndef LEVELS_H
#define LEVELS_H

#include "components.h"
#include "math.h"
#include "map.h"

typedef struct
{
    pos_t pos;
    monster_type_t type;
} npc_t;

typedef struct
{
    pos_t pos;
} chest_t;

typedef struct
{
    pos_t player;

    int room_count;
    room_t* rooms;

    int tunnel_count;
    tunnel_t* tunnels;

    int npc_count;
    npc_t* npcs;

    int chest_count;
    chest_t* chests;
} level_t;

level_t* load_level(const char* json_str);
void free_level(level_t* level);

#endif // LEVELS_H
