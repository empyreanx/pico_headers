#ifndef PICO_QT_H
#define PICO_QT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef PICO_QT_USE_DOUBLE
    typedef double qt_float;
#else
    typedef float  qt_float;
#endif

typedef uint32_t qt_value_t;

typedef struct
{
    qt_float x, y, w, h;
} qt_rect_t;

qt_rect_t qt_make_rect(qt_float x, qt_float y, qt_float w, qt_float h);

typedef struct qt_t qt_t;

#endif // PICO_QT_H

#ifdef PICO_QT_IMPLEMENTATION

#ifdef NDEBUG
    #define PICO_QT_ASSERT(expr) ((void)0)
#else
    #ifndef PICO_QT_ASSERT
        #include <assert.h>
        #define PICO_QT_ASSERT(expr) (assert(expr))
    #endif
#endif

#if !defined(PICO_QT_MALLOC) || !defined(PICO_QT_REALLOC) || !defined(PICO_QT_FREE)
#include <stdlib.h>
#define PICO_QT_MALLOC(size)       (malloc(size))
#define PICO_QT_REALLOC(ptr, size) (realloc(ptr, size))
#define PICO_QT_FREE(ptr)          (free(ptr))
#endif

#ifndef PICO_QT_MAX_DEPTH
#define PICO_QT_MAX_DEPTH 8
#endif

#define QT_ASSERT    PICO_QT_ASSERT
#define QT_MALLOC    PICO_QT_MALLOC
#define QT_REALLOC   PICO_QT_REALLOC
#define QT_FREE      PICO_QT_FREE
#define QT_MAX_DEPTH PICO_QT_MAX_DEPTH

typedef struct qt_node_t qt_node_t;

typedef struct
{
    qt_value_t value;
    qt_rect_t  bounds;
} qt_item_t;

typedef struct
{
    int        capacity;
    int        size;
    qt_item_t* items;
} qt_array_t;

struct qt_node_t
{
    int        depth;
    qt_rect_t  bounds[4];
    qt_node_t* nodes[4];
    qt_array_t items;
};

struct qt_t
{
    qt_rect_t  bounds;
    qt_node_t* root;
    qt_array_t tmp;
};

void qt_array_init(qt_array_t* array, int capacity)
{
    array->size = 0;
    array->capacity = capacity;

    if (capacity > 0)
        array->items = QT_MALLOC(capacity * sizeof(qt_item_t));
    else
        array->items = NULL;
}

void qt_array_destroy(qt_array_t* array)
{
    QT_FREE(array->items);

    array->items = NULL;
    array->capacity = 0;
    array->size = 0;
}

void qt_array_resize(qt_array_t* array, int size)
{
    if (size < array->capacity)
    {
        array->size = size;
        return;
    }

    while (array->capacity <= size)
    {
        array->capacity += (array->capacity / 2) + 2;
    }

    array->items = QT_REALLOC(array->items, array->capacity * sizeof(qt_item_t));
    array->size = size;
}

void qt_array_push(qt_array_t* array, qt_rect_t bounds, qt_value_t value)
{
    int size = array->size;

    qt_array_resize(array, size + 1);

    array->items[size].value  = value;
    array->items[size].bounds = bounds;
}

void qt_array_cat(qt_array_t* dst, qt_array_t* src)
{
    int total_capacity = dst->capacity + src->capacity;

    if (dst->capacity < total_capacity)
    {
        dst->items = QT_REALLOC(dst->items, total_capacity * sizeof(qt_item_t));
        dst->capacity = total_capacity;
    }

    memcpy(&dst->items[dst->size], src->items, src->size * sizeof(qt_item_t));

    dst->size += src->size;
}

void qt_array_remove(qt_array_t* array, int index)
{
    QT_ASSERT(index < array->size);

    int size = array->size;

    if (size > 0)
    {
        array->items[index] = array->items[size - 1];
        array->size--;
    }
}

qt_rect_t qt_make_rect(qt_float x, qt_float y, qt_float w, qt_float h)
{
    qt_rect_t r = { x, y, w, h };
    return r;
}

bool qt_rect_contains(qt_rect_t* r1, qt_rect_t* r2)
{
    return r1->x <= r2->x &&
           r1->y <= r2->y &&
           r1->x + r1->w >= r2->x + r2->w &&
           r1->y + r1->h >= r2->y + r2->h;
}

bool qt_rect_overlaps(qt_rect_t* r1, qt_rect_t* r2)
{
    return r1->x + r1->w >= r2->x &&
           r1->y + r1->h >= r2->y &&
           r2->x + r2->w >= r1->x &&
           r2->y + r2->h >= r1->y;
}

qt_node_t* qt_node_new(qt_rect_t bounds, int depth)
{
    qt_node_t* node = QT_MALLOC(sizeof(qt_node_t));

    memset(node, 0, sizeof(qt_node_t));

    node->depth = depth;

    bounds.w /= 2.0f;
    bounds.h /= 2.0f;

    node->bounds[0] = qt_make_rect(bounds.x,
                                   bounds.y,
                                   bounds.w,
                                   bounds.h);

    node->bounds[1] = qt_make_rect(bounds.x + bounds.w,
                                   bounds.y,
                                   bounds.w,
                                   bounds.h);

    node->bounds[2] = qt_make_rect(bounds.x,
                                   bounds.y + bounds.h,
                                   bounds.w,
                                   bounds.h);

    node->bounds[3] = qt_make_rect(bounds.x + bounds.w,
                                   bounds.y + bounds.h,
                                   bounds.w,
                                   bounds.h);

    qt_array_init(&node->items, 8);

    return node;
}

void qt_node_free(qt_node_t* node)
{
    for (int i = 0; i < 4; i++)
    {
        qt_node_free(node->nodes[i]);
    }

    qt_array_destroy(&node->items);

    QT_FREE(node);
}

void qt_node_insert(qt_node_t* node, qt_rect_t bounds, qt_value_t value)
{
    for (int i = 0; i < 4; i++)
    {
        if (qt_rect_contains(&node->bounds[i], &bounds))
        {
            if (node->depth + 1 < QT_MAX_DEPTH)
            {
                if (!node->nodes[i])
                {
                    node->nodes[i] = qt_node_new(node->bounds[i], node->depth + 1);
                }

                qt_node_insert(node->nodes[i], bounds, value);
                return;
            }
        }
    }

    qt_array_push(&node->items, bounds, value);
}

bool qt_node_remove(qt_node_t* node, qt_value_t value)
{
    for (int i = 0; i < node->items.size; i++)
    {
        qt_item_t* item = &node->items.items[i];

        if (item->value == value)
        {
            qt_array_remove(&node->items, i);
            return true;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        if (node->nodes[i])
        {
            if (qt_node_remove(node->nodes[i], value))
                return true;
        }
    }

    return false;
}

void qt_node_all(qt_node_t* node, qt_array_t* array)
{
    qt_array_cat(array, &node->items);

    for (int i = 0; i < 4; i++)
    {
        if (node->nodes[i])
            qt_node_all(node->nodes[i], array);
    }
}

void qt_node_query(qt_node_t* node, qt_rect_t bounds, qt_array_t* array)
{
    for (int i = 0; i < node->items.size; i++)
    {
        qt_item_t* item = &node->items.items[i];

        if (qt_rect_overlaps(&bounds, &item->bounds))
            qt_array_push(array, item->bounds, item->value);
    }

    for (int i = 0; i < 4; i++)
    {
        if (node->nodes[i])
        {
            if (qt_rect_contains(&bounds, &node->bounds[i]))
            {
                qt_node_all(node->nodes[i], array);
            }
            else
            {
                if (qt_rect_overlaps(&bounds, &node->bounds[i]))
                {
                    qt_node_query(node->nodes[i], bounds, array);
                }
            }
        }
    }
}

// qt_create
// qt_destroy
// qt_reset
// qt_insert(qt, bounds, value)
// qt_remove(qt, value)
// qt_query(qt, area, array) // or qt_value_t* qt_query(qt, area, &size)

#endif // PICO_QT_IMPLEMENTATION
