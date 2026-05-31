#ifndef MAP_H
#define MAP_H

#include "math.h"

typedef struct
{
    pos_t pos;
    dim_t dim;
} room_t;

typedef struct
{
    int corner_count;
    pos_t* corners;
} tunnel_t;

#endif // MAP_H