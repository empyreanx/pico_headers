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

typedef struct qt_t qt_t;

typedef struct
{
    qt_float x, y, w, h;
} qt_rect_t;

qt_rect_t qt_make_rect(qt_float x, qt_float y, qt_float w, qt_float h);

qt_t* qt_create(qt_rect_t bounds);
void qt_destroy(qt_t* qt);
void qt_insert(qt_t* qt, qt_rect_t bounds, qt_value_t value);
bool qt_remove(qt_t* qt, qt_value_t value);
qt_value_t* qt_query(qt_t* qt, qt_rect_t area, int* size);
void qt_free(void* ptr);

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
    qt_array_t tmp;
    qt_rect_t  bounds;
    qt_node_t* root;
};

static void qt_array_init(qt_array_t* array, int capacity);
static void qt_array_destroy(qt_array_t* array);
static void qt_array_resize(qt_array_t* array, int size);
static void qt_array_push(qt_array_t* array, qt_rect_t bounds, qt_value_t value);
static void qt_array_cat(qt_array_t* dst, qt_array_t* src);
static void qt_array_remove(qt_array_t* array, int index);
static bool qt_rect_contains(qt_rect_t* r1, qt_rect_t* r2);
static bool qt_rect_overlaps(qt_rect_t* r1, qt_rect_t* r2);
static qt_node_t* qt_node_create(qt_rect_t bounds, int depth);
static void qt_node_destroy(qt_node_t* node);
static void qt_node_insert(qt_node_t* node, qt_rect_t* bounds, qt_value_t value);
static bool qt_node_remove(qt_node_t* node, qt_value_t value);
static void qt_node_all_items(qt_node_t* node, qt_array_t* array);
static void qt_node_query(qt_node_t* node, qt_rect_t* area, qt_array_t* array);

qt_t* qt_create(qt_rect_t bounds)
{
    qt_t* qt = QT_MALLOC(sizeof(qt_t));

    qt->bounds = bounds;
    qt->root = qt_node_create(bounds, 0);
    qt_array_init(&qt->tmp, 32);

    return qt;
}

void qt_destroy(qt_t* qt)
{
    qt_array_destroy(&qt->tmp);
    qt_node_destroy(qt->root);
    QT_FREE(qt);
}

// qt_reset

void qt_insert(qt_t* qt, qt_rect_t bounds, qt_value_t value)
{
    qt_node_insert(qt->root, &bounds, value);
}

bool qt_remove(qt_t* qt, qt_value_t value)
{
    return qt_node_remove(qt->root, value);
}

qt_value_t* qt_query(qt_t* qt, qt_rect_t area, int* size)
{
    qt->tmp.size = 0;

    qt_node_query(qt->root, &area, &qt->tmp);

    qt_value_t* values = QT_MALLOC(qt->tmp.size * sizeof(qt_value_t));

    for (int i = 0; i < qt->tmp.size; i++)
    {
        values[i] = qt->tmp.items[i].value;
    }

    *size = qt->tmp.size;

    return values;
}

void qt_free(void* ptr)
{
    QT_FREE(ptr);
}

static void qt_array_init(qt_array_t* array, int capacity)
{
    array->size = 0;
    array->capacity = capacity;

    if (capacity > 0)
        array->items = QT_MALLOC(capacity * sizeof(qt_item_t));
    else
        array->items = NULL;
}

static void qt_array_destroy(qt_array_t* array)
{
    QT_FREE(array->items);

    array->items = NULL;
    array->capacity = 0;
    array->size = 0;
}

static void qt_array_resize(qt_array_t* array, int size)
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

static void qt_array_push(qt_array_t* array, qt_rect_t bounds, qt_value_t value)
{
    int size = array->size;

    qt_array_resize(array, size + 1);

    array->items[size].value  = value;
    array->items[size].bounds = bounds;
}

static void qt_array_cat(qt_array_t* dst, qt_array_t* src)
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

static void qt_array_remove(qt_array_t* array, int index)
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

static bool qt_rect_contains(qt_rect_t* r1, qt_rect_t* r2)
{
    return r1->x <= r2->x &&
           r1->y <= r2->y &&
           r1->x + r1->w >= r2->x + r2->w &&
           r1->y + r1->h >= r2->y + r2->h;
}

static bool qt_rect_overlaps(qt_rect_t* r1, qt_rect_t* r2)
{
    return r1->x + r1->w >= r2->x &&
           r1->y + r1->h >= r2->y &&
           r2->x + r2->w >= r1->x &&
           r2->y + r2->h >= r1->y;
}

static qt_node_t* qt_node_create(qt_rect_t bounds, int depth)
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

static void qt_node_destroy(qt_node_t* node)
{
    for (int i = 0; i < 4; i++)
    {
        if (node->nodes[i])
            qt_node_destroy(node->nodes[i]);
    }

    qt_array_destroy(&node->items);

    QT_FREE(node);
}

static void qt_node_insert(qt_node_t* node, qt_rect_t* bounds, qt_value_t value)
{
    for (int i = 0; i < 4; i++)
    {
        if (qt_rect_contains(&node->bounds[i], bounds))
        {
            if (node->depth + 1 < QT_MAX_DEPTH)
            {
                if (!node->nodes[i])
                {
                    node->nodes[i] = qt_node_create(node->bounds[i], node->depth + 1);
                }

                qt_node_insert(node->nodes[i], bounds, value);
                return;
            }
        }
    }

    qt_array_push(&node->items, *bounds, value);
}

static bool qt_node_remove(qt_node_t* node, qt_value_t value)
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

static void qt_node_all_items(qt_node_t* node, qt_array_t* array)
{
    qt_array_cat(array, &node->items);

    for (int i = 0; i < 4; i++)
    {
        if (node->nodes[i])
            qt_node_all_items(node->nodes[i], array);
    }
}

static void qt_node_query(qt_node_t* node, qt_rect_t* area, qt_array_t* array)
{
    for (int i = 0; i < node->items.size; i++)
    {
        qt_item_t* item = &node->items.items[i];

        if (qt_rect_overlaps(area, &item->bounds))
            qt_array_push(array, item->bounds, item->value);
    }

    for (int i = 0; i < 4; i++)
    {
        if (node->nodes[i])
        {
            if (qt_rect_contains(area, &node->bounds[i]))
            {
                qt_node_all_items(node->nodes[i], array);
            }
            else
            {
                if (qt_rect_overlaps(area, &node->bounds[i]))
                {
                    qt_node_query(node->nodes[i], area, array);
                }
            }
        }
    }
}

#endif // PICO_QT_IMPLEMENTATION
