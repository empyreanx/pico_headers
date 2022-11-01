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

typedef enum
{
    QT_VALUE_ID,
    QT_VALUE_PTR
} qt_value_type_t;

typedef struct
{
    qt_value_type_t type;

    union
    {
        qt_id_t id;
        void*   ptr;
    } value;
} qt_value_t;

typedef struct qt_t qt_t;
typedef struct qt_node_t qt_node_t;
typedef struct qt_array_t qt_array_t;

//qt_value_t qt_make_id(qt_id_t id)
//qt_value_t qt_make_ptr(void* ptr)
//qt_value_equal(qt_value_t v1, qt_value_t v2);

#endif // PICO_QT_H

#ifdef PICO_QT_IMPLEMENTATION

// PICO_QT_MAX_DEPTH
// PICO_QT_MALLOC
// PICO_QT FREE
// PICO_QT_REALLOC
// PICO_QT_MIN_ARRAY_SIZE

struct qt_array_t
{
    int size;
    int capacity;
    qt_value_t* array;
};

struct qt_t
{
    qt_node_t* root;
};

struct qt_node_t
{
    int        depth;
    qt_rect_t  bounds;
    qt_rect_t  regions[4];
    qt_node_t* children[4];
    qt_array_t items;
};

// qt_new
// qt_free
// qt_reset
// qt_insert(qt, bounds, value)
// qt_remove(qt, value)
// qt_query(qt, area, array) //or

// qt_rect_make
// qt_rect_contains
// qt_rect_overlaps

// qt_array_new
// qt_array_free
// qt_array_size
// qt_array_values

// qt_array_push
// qt_array_concat

// qt_node_new
// qt_node_free
// qt_node_insert
// qt_node_remove
// qt_node_query

#endif // PICO_QT_IMPLEMENTATION
