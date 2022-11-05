/**
    @file pico_qt.h
    @brief A simple quadtree library

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Features:
    ---------
    - Written in C99
    - Single header library for easy build system integration
    - Simple and concise API
    - Permissive license (zlib or public domain)

    Summary:
    --------
    This library builds and manages [Quadtrees](https://en.wikipedia.org/wiki/Quadtree).

    A quadtree is a data structure that can be used to perform efficient spatial
    queries. Items (values + bounds) are inserted into the tree. During this
    process, space in a quadtree is subdivided to make subsequent retrieval
    fast. Queries return values for all items that are contained within or
    overlapping a search area. In some ways, the algorithm is analogous to a
    binary search.

    Currently, values are numeric. If uintptr_t is used they can also store a
    pointer. An integer value could represent an entity ID, and array index,
    or keys for a hashtable etc...

    Usage:
    ------

    To use this library in your project, add the following

    > #define PICO_QT_IMPLEMENTATION
    > #include "pico_qt.h"

    to a source file (once), then simply include the header normally.

    Constants:
    --------
    - PICO_QT_MAX_DEPTH (default: 8)

    Must be defined before PICO_QT_IMPLEMENTATION
*/

#ifndef PICO_QT_H
#define PICO_QT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PICO_QT_USE_DOUBLE
typedef double qt_float;
#else
typedef float  qt_float;
#endif

#ifdef PICO_QT_USE_UINTPTR
/**
 * @brief Value stored in a quadtree item
 * This value could be a pointer or integer value like an entity ID, array
 * index, hashtable key, etc...
 */
typedef uintptr_t qt_value_t;
#else
/**
 * @brief Value stored in a quadtree item
 * This value could be used for an entity ID, array index, hashtable key, etc...
 */
typedef uint32_t qt_value_t;
#endif

/**
 * @brief Quadtree datatype
 */
typedef struct qt_t qt_t;

/**
 * @brief Rectangle for bounds checking
 */
typedef struct
{
    qt_float x, y, w, h;
} qt_rect_t;

/**
 * @brief Function for creating a rectangle
 */
qt_rect_t qt_make_rect(qt_float x, qt_float y, qt_float w, qt_float h);

/**
 * @brief Creates a quadtree with the specified global bounds
 * @param bounds The global bounds
 * @returns A quadtree instance
 */
qt_t* qt_create(qt_rect_t bounds);

/**
 * @brief Destroys a quadtree
 * @param qt The quadtree instance to destroy
 */
void qt_destroy(qt_t* qt);

/**
 * @brief Clears the tree of all nodes
 * @param qt The quadtree instance
 */
void qt_reset(qt_t* qt);

/**
 * @brief Inserts a value with the specified bounds into a quadtree
 * @param qt     The quadtree instance
 * @param bounds The bounds associated with the value
 * @param value  The value to store in the tree
 */
void qt_insert(qt_t* qt, qt_rect_t bounds, qt_value_t value);

/**
 * @brief Searches for and removes a value in a quadtree
 *
 * Warning: This function is very inefficient. If numerous values need to be
 * removed and reinserted it is advisable to simply rebuild the tree
 *
 * @param qt    The quadtree instance
 * @param value The value to remove
 * @returns True if the item was found, and false otherwise
 */
bool qt_remove(qt_t* qt, qt_value_t value);

/**
 * @brief Returns all values associated with items that are either overlapping
 * or contained within the search area
 * @param qt   The quadtree instance
 * @param area The search area
 * @param size The number of values returned
 * @returns The values of items contained within the search area. This value is
 * dynamically allocated and should be freed by `qt_free` after use
 */
qt_value_t* qt_query(qt_t* qt, qt_rect_t area, int* size);

/**
 * @brief Free function alias intended to be used with the output of `qt_query`
 */
void qt_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif // PICO_QT_H

#ifdef PICO_QT_IMPLEMENTATION

/*=============================================================================
 * Macros and constants
 *============================================================================*/

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

/*=============================================================================
 * Internal aliases
 *============================================================================*/

#define QT_ASSERT    PICO_QT_ASSERT
#define QT_MALLOC    PICO_QT_MALLOC
#define QT_REALLOC   PICO_QT_REALLOC
#define QT_FREE      PICO_QT_FREE
#define QT_MAX_DEPTH PICO_QT_MAX_DEPTH

/*=============================================================================
 * Internal data structures
 *============================================================================*/

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

/*=============================================================================
 * Internal function declarations
 *============================================================================*/

static void qt_array_init(qt_array_t* array, int capacity);
static void qt_array_destroy(qt_array_t* array);
static void qt_array_resize(qt_array_t* array, int size);
static void qt_array_push(qt_array_t* array, const qt_rect_t* bounds, qt_value_t value);
static void qt_array_cat(qt_array_t* dst, const qt_array_t* src);
static void qt_array_remove(qt_array_t* array, int index);
static bool qt_rect_contains(const qt_rect_t* r1, const qt_rect_t* r2);
static bool qt_rect_overlaps(const qt_rect_t* r1, const qt_rect_t* r2);
static qt_node_t* qt_node_create(qt_rect_t bounds, int depth);
static void qt_node_destroy(qt_node_t* node);
static void qt_node_insert(qt_node_t* node, const qt_rect_t* bounds, qt_value_t value);
static bool qt_node_remove(qt_node_t* node, qt_value_t value);
static void qt_node_all_items(const qt_node_t* node, qt_array_t* array);
static void qt_node_query(const qt_node_t* node, const qt_rect_t* area, qt_array_t* array);

/*=============================================================================
 * Public API implementation
 *============================================================================*/

qt_t* qt_create(qt_rect_t bounds)
{
    qt_t* qt = QT_MALLOC(sizeof(qt_t));

    if (!qt)
        return NULL;

    qt->bounds = bounds;
    qt->root = qt_node_create(bounds, 0);

    qt_array_init(&qt->tmp, 32);

    return qt;
}

void qt_destroy(qt_t* qt)
{
    QT_ASSERT(qt);

    qt_array_destroy(&qt->tmp);
    qt_node_destroy(qt->root);
    QT_FREE(qt);
}

void qt_reset(qt_t* qt)
{
    QT_ASSERT(qt);

    qt_node_destroy(qt->root);
    qt->root = qt_node_create(qt->bounds, 0);
}

void qt_insert(qt_t* qt, qt_rect_t bounds, qt_value_t value)
{
    QT_ASSERT(qt);
    qt_node_insert(qt->root, &bounds, value);
}

bool qt_remove(qt_t* qt, qt_value_t value)
{
    QT_ASSERT(qt);
    return qt_node_remove(qt->root, value);
}

qt_value_t* qt_query(qt_t* qt, qt_rect_t area, int* size)
{
    QT_ASSERT(qt);
    QT_ASSERT(size);

    // Size must be valid
    if (!size)
        return NULL;

    // Reset the internal array
    qt->tmp.size = 0;

    // Start query the root node
    qt_node_query(qt->root, &area, &qt->tmp);

    // If no results then return NULL
    if (qt->tmp.size == 0)
    {
        *size = 0;
        return NULL;
    }

    // Allocate value array
    qt_value_t* values = QT_MALLOC(qt->tmp.size * sizeof(qt_value_t));

    // Fill value array
    for (int i = 0; i < qt->tmp.size; i++)
    {
        values[i] = qt->tmp.items[i].value;
    }

    // Set size and return
    *size = qt->tmp.size;

    return values;
}

void qt_free(void* ptr)
{
    if (!ptr)
        return;

    QT_FREE(ptr);
}

/*=============================================================================
 * Internal function definitions
 *============================================================================*/

static void qt_array_init(qt_array_t* array, int capacity)
{
    QT_ASSERT(array);

    array->size = 0;
    array->capacity = capacity;

    if (capacity > 0)
        array->items = QT_MALLOC(capacity * sizeof(qt_item_t));
    else
        array->items = NULL;
}

static void qt_array_destroy(qt_array_t* array)
{
    QT_ASSERT(array);
    QT_FREE(array->items);

    array->items = NULL;
    array->capacity = 0;
    array->size = 0;
}

static void qt_array_resize(qt_array_t* array, int size)
{
    QT_ASSERT(array);

    // No change if size is below the capacity
    if (size < array->capacity)
    {
        array->size = size;
        return;
    }

    // Calculate new capacity
    while (array->capacity <= size)
    {
        array->capacity += (array->capacity / 2) + 2;
    }

    // Reallocate the array and set the new size
    array->items = QT_REALLOC(array->items, array->capacity * sizeof(qt_item_t));
    array->size = size;
}

static void qt_array_push(qt_array_t* array, const qt_rect_t* bounds, qt_value_t value)
{
    QT_ASSERT(array);
    QT_ASSERT(bounds);

    int size = array->size;

    // Resize the array by one
    qt_array_resize(array, size + 1);

    // Store new item
    array->items[size].value  =  value;
    array->items[size].bounds = *bounds;
}

static void qt_array_cat(qt_array_t* dst, const qt_array_t* src)
{
    QT_ASSERT(dst);
    QT_ASSERT(src);

    int total_capacity = dst->capacity + src->capacity;

    // Resize the array if dst capacity is less than the sum of the capacities
    // This is the most likely case.
    if (dst->capacity < total_capacity)
    {
        dst->items = QT_REALLOC(dst->items, total_capacity * sizeof(qt_item_t));
        dst->capacity = total_capacity;
    }

    // Copy the contents of the src array onto the end of the dst array
    memcpy(&dst->items[dst->size], src->items, src->size * sizeof(qt_item_t));

    // Increase the size to match
    dst->size += src->size;
}

static void qt_array_remove(qt_array_t* array, int index)
{
    QT_ASSERT(array);
    QT_ASSERT(index < array->size);

    int size = array->size;

    // Overwrites the item at the index with the item at the end of the array
    // This is fast, but it changes the order of the array, however, order
    // doesn't matter in this case
    if (size > 0)
    {
        array->items[index] = array->items[size - 1];
        array->size--;
    }
}

qt_rect_t qt_make_rect(qt_float x, qt_float y, qt_float w, qt_float h)
{
    qt_rect_t r = { x, y, w, h }; return r;
}

static bool qt_rect_contains(const qt_rect_t* r1, const qt_rect_t* r2)
{
    QT_ASSERT(r1);
    QT_ASSERT(r2);

    return r1->x <= r2->x &&
           r1->y <= r2->y &&
           r1->x + r1->w >= r2->x + r2->w &&
           r1->y + r1->h >= r2->y + r2->h;
}

static bool qt_rect_overlaps(const qt_rect_t* r1, const qt_rect_t* r2)
{
    QT_ASSERT(r1);
    QT_ASSERT(r2);

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

    // Calculate subdivided bounds
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

    // Initialize item array with default capacity
    qt_array_init(&node->items, 8);

    return node;
}

static void qt_node_destroy(qt_node_t* node)
{
    QT_ASSERT(node);

    // Recursively destroy nodes
    for (int i = 0; i < 4; i++)
    {
        if (node->nodes[i])
            qt_node_destroy(node->nodes[i]);
    }

    // Deallocate array
    qt_array_destroy(&node->items);

    // Free current node
    QT_FREE(node);
}

static void qt_node_insert(qt_node_t* node, const qt_rect_t* bounds, qt_value_t value)
{
    QT_ASSERT(node);
    QT_ASSERT(bounds);

    // Loop over child nodes
    for (int i = 0; i < 4; i++)
    {
        // Child node contains new bounds
        if (qt_rect_contains(&node->bounds[i], bounds))
        {
            // Max depth has not been reached
            if (node->depth + 1 < QT_MAX_DEPTH)
            {
                // Node doesn't exist, so create it
                if (!node->nodes[i])
                {
                    node->nodes[i] = qt_node_create(node->bounds[i], node->depth + 1);
                }

                // Recursively insert into child node
                qt_node_insert(node->nodes[i], bounds, value);
                return;
            }
        }
    }

    // If none of the children contain the bounds, or the maximum depth has been
    // reached, then the item belongs to this node
    qt_array_push(&node->items, bounds, value);
}

static bool qt_node_remove(qt_node_t* node, qt_value_t value)
{
    QT_ASSERT(node);

    // Searches the items at this level and removes the item with the specified
    // value, if found
    for (int i = 0; i < node->items.size; i++)
    {
        qt_item_t* item = &node->items.items[i];

        if (item->value == value)
        {
            qt_array_remove(&node->items, i);
            return true;
        }
    }

    // If the item wasn't found, recursively search the subtrees of this node
    for (int i = 0; i < 4; i++)
    {
        if (node->nodes[i])
        {
            if (qt_node_remove(node->nodes[i], value))
                return true;
        }
    }

    // Value wasn't found
    return false;
}

static void qt_node_all_items(const qt_node_t* node, qt_array_t* array)
{
    QT_ASSERT(node);
    QT_ASSERT(array);

    // Add all items at this level to the array
    qt_array_cat(array, &node->items);

    // Recursively add all items from in the subtree
    for (int i = 0; i < 4; i++)
    {
        if (node->nodes[i])
            qt_node_all_items(node->nodes[i], array);
    }
}

static void qt_node_query(const qt_node_t* node, const qt_rect_t* area, qt_array_t* array)
{
    QT_ASSERT(node);
    QT_ASSERT(area);
    QT_ASSERT(array);

    // Searches this level for items that intersect the area an add them to the
    // array
    for (int i = 0; i < node->items.size; i++)
    {
        qt_item_t* item = &node->items.items[i];

        if (qt_rect_overlaps(area, &item->bounds))
            qt_array_push(array, &item->bounds, item->value);
    }

    // Loop over subtrees
    for (int i = 0; i < 4; i++)
    {
        if (node->nodes[i])
        {
            // If the area contains the the entire subtree, all items in the
            // subtree match and are added to the list
            if (qt_rect_contains(area, &node->bounds[i]))
            {
                qt_node_all_items(node->nodes[i], array);
            }
            else
            {
                // Otherwise, if the area intersects the bounds of the subtree,
                // the substree is recursively searched for items
                if (qt_rect_overlaps(area, &node->bounds[i]))
                {
                    qt_node_query(node->nodes[i], area, array);
                }
            }
        }
    }
}

#endif // PICO_QT_IMPLEMENTATION

/*
    ----------------------------------------------------------------------------
    This software is available under two licenses (A) or (B). You may choose
    either one as you wish:
    ----------------------------------------------------------------------------

    (A) The zlib License

    Copyright (c) 2022 James McLean

    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software in a
    product, an acknowledgment in the product documentation would be appreciated
    but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.

    ----------------------------------------------------------------------------

    (B) Public Domain (www.unlicense.org)

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or distribute
    this software, either in source code form or as a compiled binary, for any
    purpose, commercial or non-commercial, and by any means.

    In jurisdictions that recognize copyright laws, the author or authors of
    this software dedicate any and all copyright interest in the software to the
    public domain. We make this dedication for the benefit of the public at
    large and to the detriment of our heirs and successors. We intend this
    dedication to be an overt act of relinquishment in perpetuity of all present
    and future rights to this software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
    ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
