#ifndef PICO_QT_H
#define PICO_QT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef PICO_QT_USE_DOUBLE
    typedef double qt_float;
#else
    typedef float  qt_float;
#endif

typedef struct
{
    qt_float x, y, w, h;
} qt_rect_t;

typedef uint32_t qt_id_t;

typedef union
{
    qt_id_t id;
    void*   ptr;
} qt_value_t;

typedef struct qt_t qt_t;
typedef struct qt_node_t qt_node_t;
typedef struct qt_array_t qt_array_t;

#endif // PICO_QT_H

#ifdef PICO_QT_IMPLEMENTATION

struct qt_array_t
{
    qt_value_t* array;
};

struct qt_t
{
    qt_node_t* root;
};

struct qt_node_t
{
    int depth;
    qt_rect_t  bounds;
    qt_rect_t  regions[4];
    qt_node_t* children[4];
    qt_array_t items;
};

// qt_rect_make
// qt_rect_contains
// qt_rect_overlaps

#endif // PICO_QT_IMPLEMENTATION
