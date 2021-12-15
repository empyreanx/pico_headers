#include "math.h"

#include <stdlib.h>
#include <assert.h>

int random_int(int min, int max)
{
    return rand() % (max + 1 - min) + min;
}

int sign(int num)
{
    if (num < 0)
        return -1;
    else if (num > 0)
        return 1;
    else
        return 0;
}

int abs(int num)
{
    return (num < 0) ? -num : num;
}

int max(int num1, int num2)
{
    return (num1 > num2) ? num1 : num2;
}

int distance(const pos_t* p1, const pos_t* p2)
{
    return abs(p1->x - p2->x) + abs(p1->y - p2->y);
}

bool pos_equal(const pos_t* p1, const pos_t* p2)
{
    return (p1->x == p2->x && p1->y == p2->y);
}

pos_t pos_make(int x, int y)
{
    return (pos_t){ x, y };
}

pos_t pos_add(const pos_t* p1, const pos_t* p2)
{
    return (pos_t){ p1->x + p2->x, p1->y + p2->y };
}

pos_t random_move(const pos_t* start)
{
    int dir = random_int(0, 4);

    pos_t pos = *start;

    switch (dir)
    {
        case 0: // Left
            pos.x -= 1;
            break;

        case 1: // Right
            pos.x += 1;
            break;

        case 2: // Up
            pos.y -= 1;
            break;

        case 3: // Down
            pos.y += 1;
            break;

        case 4: // Stay
            break;

        default:
            assert(false);
    }

    return pos;
}

