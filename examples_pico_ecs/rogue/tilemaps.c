#include "game.h"
#include "math.h"

#include <assert.h>
#include <curses.h>
#include <stdlib.h>

tile_t get_tile(tilemap_t* map, pos_t pos)
{
    return map->tiles[pos.y * map->dim.w + pos.x];
}

void set_tile(tilemap_t* map, pos_t pos, tile_t tile)
{
    map->tiles[pos.y * map->dim.w + pos.x] = tile;
}

tile_t make_floor_tile()
{
    tile_t tile = { '*', false };
    return tile;
}

tile_t make_tunnel_tile()
{
    tile_t tile = { '#', false };
    return tile;
}

tile_t make_horizontal_wall_tile()
{
    tile_t tile = { '-', true };
    return tile;
}

tile_t make_vertical_wall_tile()
{
    tile_t tile = { '|', true };
    return tile;
}

tilemap_t* create_tilemap(game_t* game, level_t* level)
{
    tilemap_t* map = calloc(1, sizeof(tilemap_t));
    map->dim = game->term_size;
    map->tiles = calloc(map->dim.h * map->dim.w, sizeof(tile_t));

    // Create rooms
    for (int i = 0; i < level->room_count; i++)
    {
        room_t room = level->rooms[i];

        for (int y = room.pos.y; y < room.pos.y + room.dim.h - 1; y++)
        {
            pos_t pos = pos_make(room.pos.x, y);
            set_tile(map, pos, make_vertical_wall_tile());
        }

        for (int y = room.pos.y; y < room.pos.y + room.dim.h - 1; y++)
        {
            pos_t pos = pos_make(room.pos.x + room.dim.w - 1, y);
            set_tile(map, pos, make_vertical_wall_tile());
        }

        for (int x = room.pos.x; x < room.pos.x + room.dim.w; x++)
        {
            pos_t pos = pos_make(x, room.pos.y);
            set_tile(map, pos, make_horizontal_wall_tile());
        }

        for (int x = room.pos.x; x < room.pos.x + room.dim.w; x++)
        {
            pos_t pos = pos_make(x, room.pos.y + room.dim.h - 1);
            set_tile(map, pos, make_horizontal_wall_tile());
        }

        for (int y = room.pos.y + 1; y < room.pos.y + room.dim.h - 1; y++)
        {
            for (int x = room.pos.x + 1; x < room.pos.x + room.dim.w- 1; x++)
            {
                pos_t pos = pos_make(x, y);
                set_tile(map, pos, make_floor_tile());
            }
        }
    }

    // Tunnels

    for (int i = 0; i < level->tunnel_count; i++)
    {
        tunnel_t tunnel = level->tunnels[i];

        for (int j = 0; j < tunnel.corner_count - 1; j++)
        {
            pos_t corner1 = tunnel.corners[j];
            pos_t corner2 = tunnel.corners[j + 1];

            if (corner1.x == corner2.x)
            {
                int dy = sign(corner2.y - corner1.y);

                for (int y = corner1.y; y != corner2.y; y += dy)
                {
                    pos_t pos = pos_make(corner1.x, y);
                    set_tile(map, pos, make_tunnel_tile());
                }
            }
            else if (corner1.y == corner2.y)
            {
                int dx = sign(corner2.x - corner1.x);

                for (int x = corner1.x; x != corner2.x; x += dx)
                {
                    pos_t pos = pos_make(x, corner1.y);
                    set_tile(map, pos, make_tunnel_tile());
                }
            }
            else
            {
                //assert(false);
            }


            pos_t pos = tunnel.corners[j];
            set_tile(map, pos, make_floor_tile());
        }
    }

    return map;
}

void draw_tilemap(tilemap_t* map)
{
    for (int y = 0; y < map->dim.h; y++)
    {
        for (int x = 0; x < map->dim.w; x++)
        {
            tile_t tile = get_tile(map, pos_make(x, y));
            mvprintw(y, x, "%c", tile.symbol);
        }
    }
}

bool collides_with_tile(tilemap_t* map, pos_t pos)
{
    //tile_t tile = get_tile(map, pos.y + dy, pos.x + dx);
    tile_t tile = get_tile(map, pos);

    if (tile.collidable)
        return true;

    if (0 == tile.symbol)
        return true;

    return false;
}

pos_t move_toward(tilemap_t *map, pos_t* start, pos_t* target)
{
    pos_t p1 = { start->x + 1, start->y     };
    pos_t p2 = { start->x,     start->y + 1 };
    pos_t p3 = { start->x - 1, start->y     };
    pos_t p4 = { start->x    , start->y - 1 };

    int d1 = distance(&p1, target);
    int d2 = distance(&p2, target);
    int d3 = distance(&p3, target);
    int d4 = distance(&p4, target);

    int   dist_array[] = { d1, d2, d3, d4 };
    pos_t pos_array [] = { p1, p2, p3, p4 };

    int min_index = 0;

    for (int i = 0; i < 4; i++)
    {
        //if (collides_with_tile(map, pos_array[i]))
        //    continue;

        bool collides = collides_with_tile(map, pos_array[i]);

        if (dist_array[i] <= dist_array[min_index] && !collides)
            min_index = i;
    }

    return pos_array[min_index];
}

pos_t move_away(tilemap_t *map, pos_t* start, pos_t* target)
{
    pos_t p1 = { start->x + 1, start->y     };
    pos_t p2 = { start->x,     start->y + 1 };
    pos_t p3 = { start->x - 1, start->y     };
    pos_t p4 = { start->x    , start->y - 1 };

    int d1 = distance(&p1, target);
    int d2 = distance(&p2, target);
    int d3 = distance(&p3, target);
    int d4 = distance(&p4, target);

    int   dist_array[] = { d1, d2, d3, d4 };
    pos_t pos_array [] = { p1, p2, p3, p4 };

    int max_index = 0;

    for (int i = 0; i < 4; i++)
    {
        if (collides_with_tile(map, pos_array[i]))
            continue;

        if (dist_array[i] >= dist_array[max_index])
            max_index = i;
    }

    return pos_array[max_index];
}
