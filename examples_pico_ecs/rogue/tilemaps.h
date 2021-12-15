#ifndef TILEMAP_H
#define TILEMAP_H

#include "game.h"
#include "levels.h"
#include "math.h"

struct game_s;

typedef struct
{
    char symbol;
    bool collidable;
} tile_t;

typedef struct
{
    dim_t dim;
    tile_t* tiles;

    int room_count;
    room_t* rooms;

    int tunnel_count;
    tunnel_t* tunnels;
} tilemap_t;

tilemap_t* create_tilemap(struct game_s* game, level_t* level);
tile_t get_tile(tilemap_t* map, pos_t pos);
void draw_tilemap(tilemap_t* map);

bool collides_with_tile(tilemap_t* map, pos_t pos);

pos_t move_toward(tilemap_t* map, pos_t* start, pos_t* target);
pos_t move_away(tilemap_t* map, pos_t* start, pos_t* target);

#endif // TILEMAP_H
