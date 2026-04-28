/**
    @file pico_bvh.h
    @brief A Dynamic 2D Bounding Volume Heirarchy (BVH) Library

    ---------------------------------------------------------------------------
    Licensing information at end of header
    ---------------------------------------------------------------------------

    Overview
    --------

    This header implements a dynamic 2D AABB tree for broad-phase spatial
    queries. Each inserted object becomes a leaf node, while internal nodes
    are maintained automatically to bound their descendants.

    The tree is incrementally rebalanced using SAH-based local rotations,
    which keeps query/update costs near logarithmic in typical workloads.

    Features
    --------

    - Written in C99
    - Broad-phase AABB and ray queries with early-out callbacks.
    - Dynamic insert, remove, and move operations for 2D AABBs.
    - SAH-guided sibling selection and local rotations for practical balance.
    - Configurable leaf padding to reduce reinsertion churn.
    - User-defined per-leaf payload type via `PICO_BVH_UDATA_TYPE`.

    Core Concepts

    -------------
    - Leaf handle: `bvh_insert` returns an integer ID used by `bvh_move`,
        `bvh_remove`, and accessor functions.
    - Padded bounds: leaves store an expanded AABB (tight bounds + padding)
        to reduce reinsertion churn for small object motion.
    - User payload: `bvh_udata_t` is user-defined via `PICO_BVH_UDATA_TYPE`
        (defaults to `uint64_t`).

    Typical Workflow
    ----------------

    1) Create tree
        bvh_t* tree = bvh_create();
    2) Insert and keep returned IDs
        int id = bvh_insert(tree, aabb, padding, udata);
    3) Update or remove by ID
        bvh_move(tree, id, new_aabb, padding);
        bvh_remove(tree, id);
    4) Query with callbacks
        bvh_query_aabb(tree, query, cb, ctx);
        bvh_query_ray(tree, origin, dir, t_max, cb, ctx);
    5) Destroy when done
        bvh_destroy(tree);

    Query Semantics

    ---------------
    - AABB and ray queries are broad-phase: they report overlapping leaves.
    - Callbacks may return false to stop traversal early.
    - AABBs that touch at edges are considered overlapping.

    Complexity Notes
    ----------------

    - Insert/remove/move: O(log n) expected.
    - Query: O(log n + k) expected, where k is number of reported leaves.
    - Worst-case behavior can degrade if bounds distributions are pathological.
 */
#ifndef PICO_BVH_H
#define PICO_BVH_H

#include <stdbool.h>
#include <stdint.h>

// --- User Data Type Override -------------------------------------------------

#ifndef PICO_BVH_UDATA_TYPE
    #define PICO_BVH_UDATA_TYPE uint64_t
#endif

typedef PICO_BVH_UDATA_TYPE bvh_udata_t;

// --- Math Primitives ---------------------------------------------------------

typedef struct
{
    float x;
    float y;
} bvh_vec2_t;

typedef struct
{
    bvh_vec2_t min;
    bvh_vec2_t max;
} bvh_aabb_t;

// --- Public Types ------------------------------------------------------------

#define BVH_NULL_ID (-1)

/**
 * @brief Return false to stop traversal early.
 * @param leaf_id Leaf handle for the candidate hit.
 * @param udata User payload stored in that leaf.
 * @param ctx User-provided context pointer passed through query calls.
 * @return true to continue traversal, false to stop early.
 */
typedef bool (*bvh_query_cb)(int leaf_id, bvh_udata_t udata, void* ctx);

/**
 * @brief Called for every node during bvh_walk(); depth=0 at root.
 * @param aabb Node bounds.
 * @param depth Tree depth, where 0 is the root.
 * @param is_leaf true for leaf nodes, false for internal nodes.
 * @param udata User payload for leaf nodes; unspecified for internal nodes.
 * @param ctx User-provided context pointer passed through bvh_walk.
 */
typedef void (*bvh_walk_cb)(bvh_aabb_t aabb, int depth, bool is_leaf,
                            bvh_udata_t udata, void* ctx);

/**
 * @brief BVH instance
 */
typedef struct bvh_t bvh_t;

// --- Helpers -----------------------------------------------------------------

/**
 * @brief Constructs an AABB from a position and dimensions.
 * @param x Minimum x coordinate.
 * @param y Minimum y coordinate.
 * @param w Width added to x to form max.x.
 * @param h Height added to y to form max.y.
 * @return Constructed AABB.
 */
bvh_aabb_t bvh_make_aabb(float x, float y, float w, float h);

// --- Lifecycle ---------------------------------------------------------------

/**
 * @brief Allocates and initializes a BVH instances
 * @return New BVH instance, or NULL on allocation failure.
 */
bvh_t* bvh_create(void);

/**
 * @brief Destroys and deallocates a BVH instance
 * @param tree BVH instance to destroy.
 */
void bvh_destroy(bvh_t* tree);

// --- Modification ------------------------------------------------------------

/**
 * @brief Inserts a new leaf.
 *
 * The stored AABB is padded by `padding` (pass 0 for exact fit).
 *
 * @param tree BVH instance to modify.
 * @param aabb Tight bounds for the new leaf.
 * @param padding Margin added on all sides before storing the leaf bounds.
 * @param udata User payload associated with the leaf.
 * @return A stable ID for future move/remove calls.
 */
int bvh_insert(bvh_t* tree, bvh_aabb_t aabb, float padding, bvh_udata_t udata);

/**
 * @brief Remove a leaf.
 * @param tree BVH instance to modify.
 * @param leaf_id ID previously returned by bvh_insert.
 */
void bvh_remove(bvh_t* tree, int leaf_id);

/**
 * @brief Update a leaf's AABB.
 * @param tree BVH instance to modify.
 * @param leaf_id ID previously returned by bvh_insert.
 * @param new_aabb New tight bounds for the leaf.
 * @param padding Margin added on all sides before storing the leaf bounds.
 * @return true if the tree was restructured (the old padded AABB no longer
 * contained the new tight one).
 */
bool bvh_move(bvh_t* tree, int leaf_id, bvh_aabb_t new_aabb, float padding);

// --- Queries -----------------------------------------------------------------

/**
 * @brief Queries the tree against an AABB
 * @param tree BVH instance to query.
 * @param query Query AABB in world space.
 * @param cb Callback invoked for each overlapping leaf.
 * @param ctx User-provided context pointer passed to cb.
 */
void bvh_query_aabb(const bvh_t* tree, bvh_aabb_t query,
                    bvh_query_cb cb, void* ctx);
/**
 * @brief Queries the tree against a ray
 *
 * Ray: origin + t*dir, t in [0, t_max].  Use FLT_MAX for infinite ray.
 * @param tree BVH instance to query.
 * @param origin Ray origin.
 * @param dir Ray direction; it does not need to be normalized.
 * @param t_max Maximum parametric distance along the ray.
 * @param cb Callback invoked for each overlapping leaf.
 * @param ctx User-provided context pointer passed to cb.
 */
void bvh_query_ray(const bvh_t* tree,
                   bvh_vec2_t origin, bvh_vec2_t dir, float t_max,
                   bvh_query_cb cb, void* ctx);

// --- Accessors ---------------------------------------------------------------

/**
 * @brief Returns the user data from the specified leaf node
 * @param tree BVH instance to read from.
 * @param leaf_id ID previously returned by bvh_insert.
 * @return User payload stored in that leaf.
 */
bvh_udata_t bvh_get_udata(const bvh_t* tree, int leaf_id);

/**
 * @brief Returns the enlarged bounds from the specified leaf node
 * @param tree BVH instance to read from.
 * @param leaf_id ID previously returned by bvh_insert.
 * @return Stored padded AABB for that leaf.
 */
bvh_aabb_t bvh_get_padded_aabb(const bvh_t* tree, int leaf_id);

/**
 * @brief Returns number of leaves in the tree
 * @param tree BVH instance to read from.
 * @return Number of currently allocated leaves.
 */
int bvh_get_leaf_count(const bvh_t* tree);

/**
 * @brief Depth-first walk over every node (internal + leaf).
 * @param tree BVH instance to walk.
 * @param cb Callback invoked once per node.
 * @param ctx User-provided context pointer passed to cb.
 */
void  bvh_walk(const bvh_t* tree, bvh_walk_cb cb, void* ctx);

/**
 * @brief Total surface-area cost (lower = better balanced).
 * @param tree BVH instance to evaluate.
 * @return Sum of node perimeters in the tree.
 */
float bvh_get_cost(const bvh_t* tree);

#endif // PICO_BVH_H

#ifdef PICO_BVH_IMPLEMENTATION

/*
    Design notes
    ------------
    The tree is a pool-allocated binary tree stored in a flat array.
    Every node keeps:
        • A "padded" AABB (tight AABB padded by a user-supplied padding).
        • child[0], child[1] - BVH_NULL_ID for leaves.
        • parent             - BVH_NULL_ID for the root.
        • height             - 0 for leaves, max(child heights)+1 otherwise.
        • udata              - only meaningful for leaves.

    Insertion (O(log n) expected)
    ------------------------------
    We pick the best sibling for a new leaf using the surface-area heuristic:
    the sibling that minimises the total induced cost increase walking back to
    the root.  This is the exact O(log n) algorithm described by:
    Bittner et al. "Fast, Effective BVH Updates for Animated Scenes" (2015)
    (also used by Box2D's b2DynamicTree).

    After every insertion / removal we walk back to the root and:
        1. Refit the ancestor AABBs.
        2. Apply one SAH rotation at each ancestor to reduce surface-area cost.

    Rotations (SAH balance)
    ------------------------
    At each internal node we consider swapping one of its grandchildren with
    the other child.  If any swap reduces the node's induced surface area we
    apply it.  This keeps the tree height near O(log n) without a full rebuild.
*/

#include <float.h>
#include <math.h>

#ifdef NDEBUG
    #define PICO_BVH_ASSERT(expr) ((void)0)
#else
    #ifndef PICO_BVH_ASSERT
        #include <assert.h>
        #define PICO_BVH_ASSERT(expr) (assert(expr))
    #endif
#endif

#if !defined(PICO_BVH_CALLOC)  || \
    !defined(PICO_BVH_REALLOC) || \
    !defined(PICO_BVH_FREE)
    #include <stdlib.h>
    #define PICO_BVH_CALLOC(num, size)  (calloc(num, size))
    #define PICO_BVH_REALLOC(ptr, size) (realloc(ptr, size))
    #define PICO_BVH_FREE(ptr)          (free(ptr))
#endif

#ifndef PICO_BVH_MEMSET
    #include <string.h>
    #define PICO_BVH_MEMSET memset
#endif

#ifndef PICO_BVH_MEMCPY
    #include <string.h>
    #define PICO_BVH_MEMCPY memcpy
#endif

#ifndef PICO_BVH_STACK_SIZE
    #define PICO_BVH_STACK_SIZE 1024
#endif

#define BVH_IS_LEAF(n)          ((n)->child[0] == BVH_NULL_ID)
#define BVH_INITIAL_CAPACITY    64
#define PICO_BVH_EPSILON        1e-9f

// --- Internal Node -----------------------------------------------------------

typedef struct
{
    bvh_aabb_t  aabb;
    int         parent;
    int         child[2];  // BVH_NULL_ID for leaves
    int         height;    // 0 = leaf
    bvh_udata_t udata;     // Valid only for leaves
    bool        allocated;
} bvh_node_t;

// --- Tree Structure ----------------------------------------------------------

struct bvh_t
{
    bvh_node_t* nodes;
    int         capacity;
    int         root;
    int         free_list;   // Singly-linked free list via child[0]
    int         leaf_count;
};

// --- Best-Sibling Heap Types -------------------------------------------------

typedef struct
{
    int   id;
    float inherited_cost;
} bvh_heap_entry_t;

typedef struct
{
    bvh_heap_entry_t* data;
    int               size;
    int               cap;
} bvh_min_heap_t;

// --- Forward Declarations ----------------------------------------------------

static inline bvh_aabb_t    bvh_aabb_pad(bvh_aabb_t a, float m);
static inline bvh_aabb_t    bvh_aabb_union(bvh_aabb_t a, bvh_aabb_t b);
static inline float         bvh_aabb_perimeter(bvh_aabb_t a);
static inline bool          bvh_aabb_overlaps(bvh_aabb_t a, bvh_aabb_t b);
static inline bool          bvh_aabb_contains(bvh_aabb_t outer, bvh_aabb_t inner);
static void                 bvh_grow(bvh_t* t);
static int                  bvh_alloc_node(bvh_t* t);
static void                 bvh_free_node(bvh_t* t, int id);
static void                 bvh_refit(bvh_t* t, int id);
static void                 bvh_rotate(bvh_t* t, int a_id);
static void                 bvh_refit_and_rotate(bvh_t* t, int start);
static void                 bvh_heap_push(bvh_min_heap_t* h, bvh_heap_entry_t e);
static bvh_heap_entry_t     bvh_heap_pop(bvh_min_heap_t* h);
static int                  bvh_best_sibling(bvh_t* t, bvh_aabb_t new_aabb);
static bool                 bvh_ray_aabb(bvh_vec2_t origin, bvh_vec2_t inv_dir, bvh_aabb_t aabb, float t_max);
static void                 bvh_walk_rec(const bvh_t* t, int id, int depth, bvh_walk_cb cb, void* ctx);
static float                bvh_get_cost_rec(const bvh_t* t, int id);

// --- Public: Lifecycle -------------------------------------------------------

bvh_t* bvh_create(void)
{
    bvh_t* t = (bvh_t*)PICO_BVH_CALLOC(1, sizeof(bvh_t));
    PICO_BVH_ASSERT(t);

    t->capacity = BVH_INITIAL_CAPACITY;
    t->nodes    = (bvh_node_t*)PICO_BVH_CALLOC((size_t)t->capacity, sizeof(bvh_node_t));

    PICO_BVH_ASSERT(t->nodes);

    t->root       = BVH_NULL_ID;
    t->leaf_count = 0;

    // Build free list
    for (int i = 0; i < t->capacity - 1; ++i)
    {
        t->nodes[i].child[0] = i + 1;
    }

    t->nodes[t->capacity - 1].child[0] = BVH_NULL_ID;
    t->free_list = 0;

    return t;
}

void bvh_destroy(bvh_t* t)
{
    if (!t)
    {
        return;
    }

    PICO_BVH_FREE(t->nodes);
    PICO_BVH_FREE(t);
}

// --- Public: Insert ----------------------------------------------------------

int bvh_insert(bvh_t* t, bvh_aabb_t aabb, float padding, bvh_udata_t udata)
{
    int leaf_id      = bvh_alloc_node(t);
    bvh_node_t* leaf = &t->nodes[leaf_id];
    leaf->aabb       = padding > 0.f ? bvh_aabb_pad(aabb, padding) : aabb;
    leaf->udata      = udata;
    leaf->height     = 0;
    t->leaf_count++;

    // Empty tree
    if (t->root == BVH_NULL_ID)
    {
        t->root = leaf_id;
        leaf->parent = BVH_NULL_ID;
        return leaf_id;
    }

    // Find best sibling
    int sib_id = bvh_best_sibling(t, leaf->aabb);

    // Create a new internal node to replace sibling
    int new_id           = bvh_alloc_node(t);
    bvh_node_t* new_node = &t->nodes[new_id];
    int old_parent = t->nodes[sib_id].parent;

    new_node->parent   = old_parent;
    new_node->child[0] = sib_id;
    new_node->child[1] = leaf_id;
    new_node->height   = 0; // Will be set by bvh_refit

    t->nodes[sib_id].parent  = new_id;
    t->nodes[leaf_id].parent = new_id;

    if (old_parent == BVH_NULL_ID)
    {
        t->root = new_id;
    }
    else
    {
        bvh_node_t* op = &t->nodes[old_parent];

        if (op->child[0] == sib_id)
        {
            op->child[0] = new_id;
        }
        else
        {
            op->child[1] = new_id;
        }
    }

    bvh_refit_and_rotate(t, new_id);

    return leaf_id;
}

// --- Public: Remove ----------------------------------------------------------

void bvh_remove(bvh_t* t, int leaf_id)
{
    PICO_BVH_ASSERT(leaf_id >= 0 && leaf_id < t->capacity);
    PICO_BVH_ASSERT(BVH_IS_LEAF(&t->nodes[leaf_id]));

    int parent_id = t->nodes[leaf_id].parent;
    bvh_free_node(t, leaf_id);
    t->leaf_count--;

    if (parent_id == BVH_NULL_ID)
    {
        // Was the only node
        t->root = BVH_NULL_ID;
        return;
    }

    // Find the sibling, pull it up to replace the parent
    bvh_node_t* parent = &t->nodes[parent_id];
    int sibling  = parent->child[0] == leaf_id
                 ? parent->child[1] : parent->child[0];

    int grandparent = parent->parent;
    bvh_free_node(t, parent_id);

    t->nodes[sibling].parent = grandparent;

    if (grandparent == BVH_NULL_ID)
    {
        t->root = sibling;
    }
    else
    {
        bvh_node_t* gp = &t->nodes[grandparent];

        if (gp->child[0] == parent_id)
        {
            gp->child[0] = sibling;
        }
        else
        {
            gp->child[1] = sibling;
        }

        bvh_refit_and_rotate(t, grandparent);
    }
}

// --- Public: Move ------------------------------------------------------------

bool bvh_move(bvh_t* t, int leaf_id, bvh_aabb_t new_aabb, float padding)
{
    PICO_BVH_ASSERT(BVH_IS_LEAF(&t->nodes[leaf_id]));

    bvh_aabb_t padded = padding > 0.f ? bvh_aabb_pad(new_aabb, padding) : new_aabb;

    // No restructuring needed if the padded AABB still contains the new one
    if (bvh_aabb_contains(t->nodes[leaf_id].aabb, new_aabb))
    {
        // Optionally shrink if the padded box is much bigger than needed
        bvh_aabb_t big = bvh_aabb_pad(new_aabb, padding * 4.f);

        if (bvh_aabb_contains(big, t->nodes[leaf_id].aabb))
        {
            return false;
        }
    }

    bvh_udata_t ud = t->nodes[leaf_id].udata;
    bvh_remove(t, leaf_id);

    // bvh_remove freed leaf_id (and its parent).  Ensure leaf_id sits at the
    // front of the free list so bvh_alloc_node returns it first, preserving
    // the caller's handle.
    if (t->free_list != leaf_id)
    {
        int prev = t->free_list;

        while (t->nodes[prev].child[0] != leaf_id)
        {
            prev = t->nodes[prev].child[0];
        }

        t->nodes[prev].child[0] = t->nodes[leaf_id].child[0];
        t->nodes[leaf_id].child[0] = t->free_list;
        t->free_list = leaf_id;
    }

    int new_id = bvh_insert(t, padded, 0.f, ud);
    (void)new_id;
    PICO_BVH_ASSERT(new_id == leaf_id);

    return true;
}

// --- Public: Query (AABB) ----------------------------------------------------

// Iterative DFS using an explicit stack to avoid recursion overhead.
void bvh_query_aabb(const bvh_t* t, bvh_aabb_t query, bvh_query_cb cb, void* ctx)
{
    if (t->root == BVH_NULL_ID)
    {
        return;
    }

    // Stack: fixed-size.  2*height+2 suffices for a balanced tree.
    // We allocate generously; could also be dynamic.
    int stack[PICO_BVH_STACK_SIZE];
    int top = 0;
    stack[top++] = t->root;

    while (top > 0)
    {
        int id = stack[--top];
        if (id == BVH_NULL_ID)
        {
            continue;
        }

        const bvh_node_t* n = &t->nodes[id];

        if (!bvh_aabb_overlaps(n->aabb, query))
        {
            continue;
        }

        if (BVH_IS_LEAF(n))
        {
            if (!cb(id, n->udata, ctx))
            {
                return;
            }
        }
        else
        {
            PICO_BVH_ASSERT(top + 2 <= PICO_BVH_STACK_SIZE);
            stack[top++] = n->child[0];
            stack[top++] = n->child[1];
        }
    }
}

// --- Public: Query (Ray) -----------------------------------------------------

void bvh_query_ray(const bvh_t* t,
                   bvh_vec2_t origin, bvh_vec2_t dir, float t_max,
                   bvh_query_cb cb, void* ctx)
{
    if (t->root == BVH_NULL_ID)
    {
        return;
    }

    // Precompute reciprocal direction (handle zero-components)
    bvh_vec2_t inv_dir =
    {
        fabsf(dir.x) > PICO_BVH_EPSILON ? 1.f / dir.x : (dir.x >= 0.f ? FLT_MAX : -FLT_MAX),
        fabsf(dir.y) > PICO_BVH_EPSILON ? 1.f / dir.y : (dir.y >= 0.f ? FLT_MAX : -FLT_MAX)
    };

    int stack[PICO_BVH_STACK_SIZE]; // TODO: define constant
    int top = 0;
    stack[top++] = t->root;

    while (top > 0)
    {
        int id = stack[--top];
        if (id == BVH_NULL_ID)
        {
            continue;
        }

        const bvh_node_t* n = &t->nodes[id];

        if (!bvh_ray_aabb(origin, inv_dir, n->aabb, t_max))
        {
            continue;
        }

        if (BVH_IS_LEAF(n))
        {
            if (!cb(id, n->udata, ctx))
            {
                return;
            }
        }
        else
        {
            PICO_BVH_ASSERT(top + 2 <= PICO_BVH_STACK_SIZE);
            stack[top++] = n->child[0];
            stack[top++] = n->child[1];
        }
    }
}

// --- Public: Accessors -------------------------------------------------------

bvh_udata_t bvh_get_udata(const bvh_t* t, int leaf_id)
{
    PICO_BVH_ASSERT(BVH_IS_LEAF(&t->nodes[leaf_id]));
    return t->nodes[leaf_id].udata;
}

bvh_aabb_t bvh_get_padded_aabb(const bvh_t* t, int leaf_id)
{
    return t->nodes[leaf_id].aabb;
}

int bvh_get_leaf_count(const bvh_t* t) { return t->leaf_count; }

// --- Public: Walk ------------------------------------------------------------

void bvh_walk(const bvh_t* t, bvh_walk_cb cb, void* ctx)
{
    bvh_walk_rec(t, t->root, 0, cb, ctx);
}

// --- Public: Cost ------------------------------------------------------------

float bvh_get_cost(const bvh_t* t)
{
    return bvh_get_cost_rec(t, t->root);
}

// --- Math Primitives ---------------------------------------------------------

bvh_aabb_t bvh_make_aabb(float x, float y, float w, float h)
{
    return (bvh_aabb_t){ {x, y}, {x + w, y + h} };
}

static inline bvh_aabb_t bvh_aabb_pad(bvh_aabb_t a, float m)
{
    return (bvh_aabb_t){ {a.min.x - m, a.min.y - m}, {a.max.x + m, a.max.y + m} };
}

static inline bvh_aabb_t bvh_aabb_union(bvh_aabb_t a, bvh_aabb_t b)
{
    return (bvh_aabb_t){
        { a.min.x < b.min.x ? a.min.x : b.min.x,
          a.min.y < b.min.y ? a.min.y : b.min.y },
        { a.max.x > b.max.x ? a.max.x : b.max.x,
          a.max.y > b.max.y ? a.max.y : b.max.y }
    };
}

static inline float bvh_aabb_perimeter(bvh_aabb_t a)
{
    return 2.f * ((a.max.x - a.min.x) + (a.max.y - a.min.y));
}

static inline bool bvh_aabb_overlaps(bvh_aabb_t a, bvh_aabb_t b)
{
    return a.min.x <= b.max.x && a.max.x >= b.min.x
        && a.min.y <= b.max.y && a.max.y >= b.min.y;
}

static inline bool bvh_aabb_contains(bvh_aabb_t outer, bvh_aabb_t inner)
{
    return outer.min.x <= inner.min.x && inner.max.x <= outer.max.x
        && outer.min.y <= inner.min.y && inner.max.y <= outer.max.y;
}

// --- Node Pool ---------------------------------------------------------------

static void bvh_grow(bvh_t* t)
{
    int old_cap  = t->capacity;
    int new_cap  = old_cap * 2;
    t->nodes     = (bvh_node_t*)PICO_BVH_REALLOC(t->nodes, (size_t)new_cap * sizeof(bvh_node_t));

    PICO_BVH_ASSERT(t->nodes);

    PICO_BVH_MEMSET(t->nodes + old_cap, 0, (size_t)(new_cap - old_cap) * sizeof(bvh_node_t));

    // Chain new slots onto the free list
    for (int i = old_cap; i < new_cap - 1; ++i)
    {
        t->nodes[i].child[0] = i + 1;
    }

    t->nodes[new_cap - 1].child[0] = t->free_list;
    t->free_list = old_cap;
    t->capacity  = new_cap;
}

static int bvh_alloc_node(bvh_t* t)
{
    if (t->free_list == BVH_NULL_ID)
    {
        bvh_grow(t);
    }

    int id        = t->free_list;
    t->free_list  = t->nodes[id].child[0];
    bvh_node_t* n = &t->nodes[id];
    n->parent     = BVH_NULL_ID;
    n->child[0]   = BVH_NULL_ID;
    n->child[1]   = BVH_NULL_ID;
    n->height     = 0;
    n->udata      = 0;
    n->allocated  = true;
    return id;
}

static void bvh_free_node(bvh_t* t, int id)
{
    PICO_BVH_ASSERT(id >= 0 && id < t->capacity);
    t->nodes[id].allocated = false;
    t->nodes[id].child[0]  = t->free_list;
    t->free_list           = id;
}

// --- Helpers -----------------------------------------------------------------

static void bvh_refit(bvh_t* t, int id)
{
    bvh_node_t* n = &t->nodes[id];
    n->aabb       = bvh_aabb_union(t->nodes[n->child[0]].aabb,
                    t->nodes[n->child[1]].aabb);
    int h0        = t->nodes[n->child[0]].height;
    int h1        = t->nodes[n->child[1]].height;
    n->height     = 1 + (h0 > h1 ? h0 : h1);
}

// --- SAH Rotation ------------------------------------------------------------
/*
    Local tree at A before rotation:

                     A
                   /   \
                  B     C
                 / \   / \
               b0  b1 c0  c1

    We evaluate 4 one-step rotations (swapping a grandchild with the opposite
    subtree) and keep only the one that reduces perimeter(A) the most. Only two
    cases are considered for brevity.

    Candidate 0 (costs[0]): swap b0 <-> C

                     A
                   /   \
                  B     b0
                 / \
                C  b1
               / \
             c0  c1


    Candidate 2 (costs[2]): swap c0 <-> B

                     A
                   /   \
                  c0    C
                       / \
                      B  c1
                     / \
                   b0  b11

    We evaluate all valid candidates and apply the one that gives the largest
    reduction in perimeter(A). If none improves cost, no rotation is applied.
 */
static void bvh_rotate(bvh_t* t, int a_id)
{
    bvh_node_t* A = &t->nodes[a_id];

    if (A->height < 2)
    {
        return;
    }

    int b_id = A->child[0];
    int c_id = A->child[1];
    bvh_node_t* B  = &t->nodes[b_id];
    bvh_node_t* C  = &t->nodes[c_id];

    float base_cost = bvh_aabb_perimeter(A->aabb);

    // Candidate costs
    float costs[4] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };

    if (!BVH_IS_LEAF(B))
    {
        costs[0] = bvh_aabb_perimeter(bvh_aabb_union(C->aabb,
                       t->nodes[B->child[1]].aabb));  // Swap b0 <-> C
        costs[1] = bvh_aabb_perimeter(bvh_aabb_union(C->aabb,
                       t->nodes[B->child[0]].aabb));  // Swap b1 <-> C
    }

    if (!BVH_IS_LEAF(C))
    {
        costs[2] = bvh_aabb_perimeter(bvh_aabb_union(B->aabb,
                       t->nodes[C->child[1]].aabb)); // Swap c0 <-> B
        costs[3] = bvh_aabb_perimeter(bvh_aabb_union(B->aabb,
                       t->nodes[C->child[0]].aabb)); // Swap c1 <-> B
    }

    // Find best candidate
    int best = -1;
    float best_cost = base_cost;

    for (int i = 0; i < 4; ++i)
    {
        if (costs[i] < best_cost)
        {
            best_cost = costs[i];
            best = i;
        }
    }

    if (best < 0)
    {
        return;  // No improvement
    }

    switch (best)
    {
        case 0:
        { // Swap B->child[0] <-> C
            int x = B->child[0];
            B->child[0]           = c_id;
            t->nodes[c_id].parent = b_id;
            A->child[1]           = x;
            t->nodes[x].parent    = a_id;
            bvh_refit(t, b_id);
            bvh_refit(t, a_id);
            break;
        }

        case 1:
        { // Swap B->child[1] <-> C
            int x = B->child[1];
            B->child[1]           = c_id;
            t->nodes[c_id].parent = b_id;
            A->child[1]           = x;
            t->nodes[x].parent    = a_id;
            bvh_refit(t, b_id);
            bvh_refit(t, a_id);
            break;
        }

        case 2:
        { // Swap C->child[0] <-> B
            int x = C->child[0];
            C->child[0]           = b_id;
            t->nodes[b_id].parent = c_id;
            A->child[0]           = x;
            t->nodes[x].parent    = a_id;
            bvh_refit(t, c_id);
            bvh_refit(t, a_id);
            break;
        }

        case 3:
        { // Swap C->child[1] <-> B
            int x = C->child[1];
            C->child[1]           = b_id;
            t->nodes[b_id].parent = c_id;
            A->child[0]           = x;
            t->nodes[x].parent    = a_id;
            bvh_refit(t, c_id);
            bvh_refit(t, a_id);
            break;
        }
    }
}

// Walk from `start` toward the root: bvh_refit + bvh_rotate each ancestor.
static void bvh_refit_and_rotate(bvh_t* t, int start)
{
    int id = start;
    while (id != BVH_NULL_ID)
    {
        if (!BVH_IS_LEAF(&t->nodes[id]))
        {
            bvh_refit(t, id);
        }

        bvh_rotate(t, id);
        id = t->nodes[id].parent;
    }
}

// --- Best-Sibling Search (SAH) -----------------------------------------------
/*
    Branch-and-bound traversal to find the node that, when used as a sibling
    for the new leaf nl, minimises the total induced cost increase up to root.

    induced_cost(node) = cost(union(node, nl)) - cost(node)
                       + sum of [cost(union(anc, nl)) - cost(anc)] for each ancestor

    We maintain a priority queue (simple binary heap) keyed on a lower bound
    of the induced cost to prune branches early.
 */
static void bvh_heap_push(bvh_min_heap_t* h, bvh_heap_entry_t entry)
{
    if (h->size == h->cap)
    {
        h->cap  = h->cap ? h->cap * 2 : 16;
        h->data = (bvh_heap_entry_t*)PICO_BVH_REALLOC(h->data,
                                    (size_t)h->cap * sizeof(bvh_heap_entry_t));

        PICO_BVH_ASSERT(h->data);
    }

    PICO_BVH_ASSERT(h->size < h->cap);

    // Sift-up
    int i = h->size++;

    while (i > 0)
    {
        int parent = (i - 1) / 2;

        if (h->data[parent].inherited_cost <= entry.inherited_cost)
        {
            break;
        }

        h->data[i] = h->data[parent];
        i = parent;
    }

    h->data[i] = entry;
}

static bvh_heap_entry_t bvh_heap_pop(bvh_min_heap_t* h)
{
    PICO_BVH_ASSERT(h->size > 0);

    bvh_heap_entry_t top  = h->data[0];
    bvh_heap_entry_t last = h->data[--h->size];

    if (h->size == 0)
    {
        return top;
    }

    int i = 0;

    while (true)
    {
        int left = i * 2 + 1;

        if (left >= h->size)
        {
            break;
        }

        int right = left + 1;
        int child = left;

        if (right < h->size
            && h->data[right].inherited_cost < h->data[left].inherited_cost)
        {
            child = right;
        }

        if (h->data[child].inherited_cost >= last.inherited_cost)
        {
            break;
        }

        h->data[i] = h->data[child];
        i = child;
    }

    h->data[i] = last;
    return top;
}

static int bvh_best_sibling(bvh_t* t, bvh_aabb_t new_aabb)
{
    float new_cost = bvh_aabb_perimeter(new_aabb);

    bvh_min_heap_t heap = {0};
    bvh_heap_push(&heap, (bvh_heap_entry_t){ t->root, 0.f });

    int   best_id   = t->root;
    float best_cost = FLT_MAX;

    while (heap.size > 0)
    {
        bvh_heap_entry_t entry = bvh_heap_pop(&heap);

        if (entry.inherited_cost >= best_cost)
        {
            break;  // Prune
        }

        bvh_node_t* node      = &t->nodes[entry.id];
        bvh_aabb_t  combined  = bvh_aabb_union(node->aabb, new_aabb);
        float direct_cost = bvh_aabb_perimeter(combined);
        float total_cost  = direct_cost + entry.inherited_cost;

        if (total_cost < best_cost)
        {
            best_cost = total_cost;
            best_id   = entry.id;
        }

        if (!BVH_IS_LEAF(node))
        {
            // Inherited cost that any descendant must pay
            float inherited_cost = entry.inherited_cost + direct_cost
                                 - bvh_aabb_perimeter(node->aabb);

            float lower_bound = inherited_cost + new_cost; // <= P(union(child, new)) + inherited

            if (lower_bound < best_cost)
            {
                bvh_heap_push(&heap, (bvh_heap_entry_t){ node->child[0], inherited_cost });
                bvh_heap_push(&heap, (bvh_heap_entry_t){ node->child[1], inherited_cost });
            }
        }
    }

    PICO_BVH_FREE(heap.data); // TODO: store heap in bvh_t
    return best_id;
}

// --- Ray Test ----------------------------------------------------------------

//Slab test for ray vs AABB intersection.
//Returns true if the ray hits the AABB within [0, t_max].
static bool bvh_ray_aabb(bvh_vec2_t origin, bvh_vec2_t inv_dir, bvh_aabb_t aabb, float t_max)
{
    float tx1 = (aabb.min.x - origin.x) * inv_dir.x;
    float tx2 = (aabb.max.x - origin.x) * inv_dir.x;
    float tmin = tx1 < tx2 ? tx1 : tx2;
    float tmax = tx1 > tx2 ? tx1 : tx2;

    float ty1 = (aabb.min.y - origin.y) * inv_dir.y;
    float ty2 = (aabb.max.y - origin.y) * inv_dir.y;
    float tymin = ty1 < ty2 ? ty1 : ty2;
    float tymax = ty1 > ty2 ? ty1 : ty2;

    tmin = tmin > tymin ? tmin : tymin;
    tmax = tmax < tymax ? tmax : tymax;

    return tmax >= 0.f && tmin <= tmax && tmin <= t_max;
}

// --- Walk Helper -------------------------------------------------------------

static void bvh_walk_rec(const bvh_t* t, int id, int depth, bvh_walk_cb cb, void* ctx)
{
    if (id == BVH_NULL_ID)
    {
        return;
    }

    const bvh_node_t* n = &t->nodes[id];
    cb(n->aabb, depth, BVH_IS_LEAF(n), n->udata, ctx);

    if (!BVH_IS_LEAF(n))
    {
        bvh_walk_rec(t, n->child[0], depth + 1, cb, ctx);
        bvh_walk_rec(t, n->child[1], depth + 1, cb, ctx);
    }
}

// --- Cost Helper -------------------------------------------------------------

static float bvh_get_cost_rec(const bvh_t* t, int id)
{
    if (id == BVH_NULL_ID)
    {
        return 0.f;
    }

    const bvh_node_t* n = &t->nodes[id];
    float c = bvh_aabb_perimeter(n->aabb);

    if (!BVH_IS_LEAF(n))
    {
        c += bvh_get_cost_rec(t, n->child[0]) + bvh_get_cost_rec(t, n->child[1]);
    }

    return c;
}

#endif // PICO_BVH_IMPLEMENTATION

/*
    ---------------------------------------------------------------------------
    This software is available under two licenses (A) or (B). You may choose
    either one as you wish:
    ---------------------------------------------------------------------------

    (A) The MIT License

    Copyright (c) 2026 James McLean

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.

    ---------------------------------------------------------------------------

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
