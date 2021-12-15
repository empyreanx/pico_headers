#ifndef MATH_H
#define MATH_H

#include <stdbool.h>

typedef struct
{
    int x;
    int y;
} pos_t;

typedef struct
{
    int h;
    int w;
} dim_t;

int random_int(int min, int max);
int sign(int num);
int abs(int num);
int max(int num1, int num2);

int distance(const pos_t* p1, const pos_t* p2);

pos_t pos_make(int x, int y);
pos_t pos_add(const pos_t* p1, const pos_t* p2);
bool pos_equal(const pos_t* p1, const pos_t* p2);

pos_t random_move(const pos_t* start);

#endif // MATH_H
