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
    overlap the search area.

    Currently, values are numeric. If uintptr_t is used they can also store a
    pointer. An integer value could represent an entity ID, an array index, a
    key for a hashtable etc...

    Usage:
    ------

    To use this library in your project, add the following

    > #define PICO_QT_IMPLEMENTATION
    > #include "pico_qt.h"

    to a source file (once), then simply include the header normally.

    Constants:
    --------
    - PICO_QT_MAX_DEPTH (default: 8)
    - PICO_QT_MIN_NODE_CAPACITY (default: 16)
    - PICO_QT_MIN_CAPACITY (default: 256)

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
/**
 * @brief Double precision floating point type
 */
typedef double qt_float;
#else
/**
 * @brief Single precision floating point type
 */
typedef float  qt_float;
#endif

#ifdef PICO_QT_USE_UINTPTR
/**
 * @brief Value data type that can store an integer or pointer
 */
typedef uintptr_t qt_value_t;
#else
/**
 * @brief Value data type that can store an integer
 */
typedef uint32_t qt_value_t;
#endif

/**
 * @brief Quadtree data structure
 */
typedef struct qt_t qt_t;

/**
 * @brief Rectangle for representing bounds
 */
typedef struct
{
    qt_float x, y, w, h;
} qt_rect_t;

/**
 * @brief Utility function for creating a rectangle
 */
qt_rect_t qt_make_rect(qt_float x, qt_float y, qt_float w, qt_float h);

/**
 * @brief Creates a quadtree with the specified global bounds
 *
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
 * @brief Removes all nodes in the tree
 * @param qt The quadtree instance
 */
void qt_reset(qt_t* qt);

/**
 * @brief Inserts a value with the specified bounds into a quadtree
 *
 * @param qt     The quadtree instance
 * @param bounds The bounds associated with the value
 * @param value  The value to store in the tree
 */
void qt_insert(qt_t* qt, qt_rect_t bounds, qt_value_t value);

/**
 * @brief Searches for and removes a value in a quadtree
 *
 * This function is very inefficient. If numerous values need to be removed and
 * reinserted it is advisable to simply rebuild the tree.
 *
 * @param qt    The quadtree instance
 * @param value The value to remove
 * @returns True if the item was found, and false otherwise
 */
bool qt_remove(qt_t* qt, qt_value_t value);

/**
 * @brief Returns all values associated with items that are either overlapping
 * or contained within the search area
 *
 * @param qt   The quadtree instance
 * @param area The search area
 * @param size The number of values returned
 *
 * @returns The values of items contained within the search area. This array is
 * dynamically allocated and should be deallocated by using `qt_free` after use
 */
qt_value_t* qt_query(qt_t* qt, qt_rect_t area, int* size);

/**
 * @brief Function for deallocating the output of `qt_query`
 */
void qt_free(qt_value_t* array);

/**
 * @brief Removes all items in the tree
 *
 * This function preserves the internal structure of the tree making it much
 * faster than `qt_reset`. Reinserting values is probably faster too, however,
 * repeated calls to this function may result in fragmentation of the tree. The
 * function `qt_clean` can repair this fragmentation, however, it is expensive.
 *
 * @param qt The quadtree instance
 */
void qt_clear(qt_t* qt);

/**
 * @brief Resets the tree and reinserts all items
 *
 * This function can repair fragmentation resulting from repeated use of
 * `qt_remove` or `qt_clear`. This is an expensive operation since it must
 * extract all of the items from the tree, remove all of the nodes, and then
 * reinsert all of the items.
 *
 * @param qt The quadtree instance
 */
void qt_clean(qt_t* qt);

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

// Maximum depth of a quadtree:
// There is a tradeoff between space and time complexity. Lower values have
// smaller space requirements, but higher search times. Indeed, a tree with
// a max depth of zero reduces to a linear bounds check. Higher values speed up
// searches, but at the cost of increased space. There are, however, diminishing
// returns with regard to increasing the max depth too much. Eventually all of
// the space is wasted with no benefit to performance. The default value strikes
// a balance between the two, however it may need to be increased or decreased
// somewhat if the global bounds is particularly large or small, respectively.
#ifndef PICO_QT_MAX_DEPTH
#define PICO_QT_MAX_DEPTH 6
#endif

// Minimum capacity of array containing node items
#ifndef PICO_QT_MIN_NODE_CAPACITY
#define PICO_QT_MIN_NODE_CAPACITY 16
#endif

// Minimum capacity of quadtree's internal array
#ifndef PICO_QT_MIN_CAPACITY
#define PICO_QT_MIN_CAPACITY 256
#endif

/*=============================================================================
 * Internal aliases
 *============================================================================*/

#define QT_ASSERT            PICO_QT_ASSERT
#define QT_MALLOC            PICO_QT_MALLOC
#define QT_REALLOC           PICO_QT_REALLOC
#define QT_FREE              PICO_QT_FREE
#define QT_MAX_DEPTH         PICO_QT_MAX_DEPTH
#define QT_MIN_NODE_CAPACITY PICO_QT_MIN_NODE_CAPACITY
#define QT_MIN_CAPACITY      PICO_QT_MIN_CAPACITY

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
static void qt_node_clear(qt_node_t* node);

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

    qt_array_init(&qt->tmp, QT_MIN_CAPACITY);

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

void qt_free(qt_value_t* array)
{
    if (!array)
        return;

    QT_FREE(array);
}


void qt_clear(qt_t* qt)
{
    QT_ASSERT(qt);
    qt_node_clear(qt->root);
}

void qt_clean(qt_t* qt)
{
    QT_ASSERT(qt);

    qt_array_t array;
    qt_array_init(&array, QT_MIN_CAPACITY);

    qt_node_all_items(qt->root, &array);
    qt_reset(qt);

    for (int i = 0; i < array.size; i++)
    {
        qt_item_t* item = &array.items[i];
        qt_insert(qt, item->bounds, item->value);
    }

    qt_array_destroy(&array);
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
    // This is the most likely case
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
    // This is fast, but it changes the order of the array Fortunately, order
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

    // Calculates bounds of subtrees
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
    qt_array_init(&node->items, QT_MIN_NODE_CAPACITY);

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

    // Deallocate item array
    qt_array_destroy(&node->items);

    // Free current node
    QT_FREE(node);
}

static void qt_node_insert(qt_node_t* node, const qt_rect_t* bounds, qt_value_t value)
{
    QT_ASSERT(node);
    QT_ASSERT(bounds);

    // The purpose of this function is to optimally fit the item into a subtree.
    // This occurs when the item is no longer fully contained within a subtree,
    // or the depth limit has been reached.

    // Checks to see if the depth limit has been reached. If it hasn't, try to
    // fit the item into a subtree
    if (node->depth + 1 < QT_MAX_DEPTH)
    {
        // Loop over child nodes
        for (int i = 0; i < 4; i++)
        {
            // Check if subtree contains the bounds
            if (qt_rect_contains(&node->bounds[i], bounds))
            {
                // If child node does not exist, then create it
                if (!node->nodes[i])
                {
                    node->nodes[i] = qt_node_create(node->bounds[i], node->depth + 1);
                }

                // Recursively try to insert the item into the subtree
                qt_node_insert(node->nodes[i], bounds, value);
                return;
            }
        }
    }

    // If none of the children fully contain the bounds, or the maximum depth
    // has been reached, then the item belongs to this node
    qt_array_push(&node->items, bounds, value);
}

static bool qt_node_remove(qt_node_t* node, qt_value_t value)
{
    QT_ASSERT(node);

    // Searches the items in this node and, if found, removes the item with the
    // specified value
    for (int i = 0; i < node->items.size; i++)
    {
        qt_item_t* item = &node->items.items[i];

        // If value is found, then remove the node from the items array
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

    // Add all items in this node into the array
    qt_array_cat(array, &node->items);

    // Recursively add all items found in the subtrees
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

    // Searches for items in this node that intersect the area and adds them to
    // the array
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
            // subtree match and are recursively added to the array
            if (qt_rect_contains(area, &node->bounds[i]))
            {
                qt_node_all_items(node->nodes[i], array);
            }
            else
            {
                // Otherwise, if the area intersects the bounds of the subtree,
                // the subtree is recursively searched for items intersecting
                // or contained within the area
                if (qt_rect_overlaps(area, &node->bounds[i]))
                {
                    qt_node_query(node->nodes[i], area, array);
                }
            }
        }
    }
}

static void qt_node_clear(qt_node_t* node)
{
    node->items.size = 0;

    for (int i = 0; i < 4; i++)
    {
        qt_node_clear(node->nodes[i]);
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
