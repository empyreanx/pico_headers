#ifndef PICO_QT_H
#define PICO_QT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef PICO_QT_USE_DOUBLE
    typedef double qt_float;
#else
    typedef float  qt_float;
#endif

typedef uint32_t qt_id_t;

typedef struct
{
    qt_float x, y, w, h;
} qt_rect_t;

typedef struct qt_t qt_t;
typedef struct qt_node_t qt_node_t;

#endif // PICO_QT_H

#ifdef PICO_QT_IMPLEMENTATION

// PICO_QT_MAX_DEPTH
// PICO_QT_MALLOC
// PICO_QT FREE
// PICO_QT_REALLOC
// PICO_QT_ASSERT

typedef struct qt_array_t qt_array_t; // Array of ids

typedef struct
{
    qt_rect_t bounds;
    qt_id_t id;
} qt_node_item;

// Don't expose array externally mean I need a qt_free function
struct qt_array_t
{
    int size;
    int capacity;
    qt_node_item* array;
};

struct qt_t
{
    qt_node_t* root;
};

struct qt_node_t
{
    int        depth;
    qt_rect_t  bounds;
    qt_rect_t  child_bounds[4];
    qt_node_t* child_nodes[4];
    qt_array_t items;
};

// qt_create
// qt_destroy
// qt_reset
// qt_insert(qt, bounds, value)
// qt_remove(qt, value)
// qt_query(qt, area, array) // or qt_id_t* qt_query(qt, area, &size)

// qt_rect_make
// qt_rect_contains
// qt_rect_overlaps

// qt_array_new(capacity)
// qt_array_free(array)
// qt_array_size(array)
// qt_array_values(array)
// qt_array_resize
// qt_array_push
// qt_array_concat

// qt_node_new
// qt_node_free
// qt_node_insert
// qt_node_remove
// qt_node_query

#endif // PICO_QT_IMPLEMENTATION

/*typedef enum
{
    QT_VALUE_ID,
    QT_VALUE_PTR
} qt_type_t;

typedef struct
{
    qt_type_t type;

    union
    {
        qt_id_t id;
        void*   ptr;
    } value;

} qt_value_t;*/

//qt_value_t qt_make_id(qt_id_t id)
//qt_value_t qt_make_ptr(void* ptr)
//qt_value_equal(qt_value_t v1, qt_value_t v2);
