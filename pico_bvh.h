/**
 * @file pico_bvh.h A 2D Dynamic AABB-tree Bounding Volume Heirarchy (BVH)
 *
 * A self-balancing bounding volume hierarchy.  Leaves hold user objects;
 * internal nodes are managed automatically.  The tree stays balanced via
 * surface-area-heuristic (SAH) rotations after every insert / remove.
 *
 * Typical usage:
 *
 *   BVH* tree = bvh_create();
 *
 *   int id = bvh_insert(tree, aabb, margin, user_data);
 *   bvh_move  (tree, id, new_aabb, margin);
 *   bvh_remove(tree, id);
 *
 *   bvh_query_aabb(tree, query, cb, ctx);
 *   bvh_query_ray (tree, origin, dir, max_t, cb, ctx);
 *
 *   bvh_destroy(tree);
 */
#ifndef PICO_BVH_H
#define PICO_BVH_H

#include <stdbool.h>

/* ── math primitives ─────────────────────────────────────────────────────── */

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

/* ── public types ────────────────────────────────────────────────────────── */

#define BVH_NULL_ID (-1)

/**
 * @brief Return false to stop traversal early.
 */
typedef bool (*bvh_query_cb)(int leaf_id, void *user_data, void *ctx);

/**
 * @brief Called for every node during bvh_walk(); depth=0 at root.
 */
typedef void (*bvh_walk_cb)(bvh_aabb_t aabb, int depth, bool is_leaf,
                            void *user_data, void *ctx);

/**
 * @brief BVH instance
 */
typedef struct bvh_t bvh_t;

/* ── lifecycle ───────────────────────────────────────────────────────────── */

/**
 * @brief Allocates and initializes a BVH instances
 */
bvh_t *bvh_create(void);

/**
 * @brief Destroys and deallocates a BVH instance
 */
void bvh_destroy(bvh_t* tree);

/* ── modification ────────────────────────────────────────────────────────── */

/**
 * @brief Inserts a new leaf.
 *
 * The stored AABB is padded by `margin` (pass 0 for exact fit).
 *
 * @returns A stable ID for future move/remove calls.
 */
int  bvh_insert(bvh_t* tree, bvh_aabb_t aabb, float margin, void *user_data);

/**
 * @brief Remove a leaf.
 */
void bvh_remove(bvh_t* tree, int leaf_id);

/**
 * @brief Update a leaf's AABB.
 * @returns true if the tree was restructured (the old padded AABB no longer
 * contained the new tight one).
 */
bool bvh_move(bvh_t* tree, int leaf_id, bvh_aabb_t new_aabb, float margin);

/* ── queries ─────────────────────────────────────────────────────────────── */

/**
 * @brief Queries the tree against an AABB
 */
void bvh_query_aabb(const bvh_t* tree, bvh_aabb_t query,
                    bvh_query_cb cb, void *ctx);
/**
 * @brief Queries the tree against a ray
 *
 * Ray: origin + t*dir, t in [0, max_t].  Use FLT_MAX for infinite ray.
 */
void bvh_query_ray(const bvh_t* tree,
                   bvh_vec2_t origin, bvh_vec2_t dir, float max_t,
                   bvh_query_cb cb, void *ctx);

/* ── accessors ───────────────────────────────────────────────────────────── */

/**
 * @brief Returns the user data from the specified leaf node
 */
void* bvh_user_data  (const bvh_t* tree, int leaf_id);

/**
 * @brief Returns the enlarged bounds from the specified leaf node
 */
bvh_aabb_t bvh_padded_aabb   (const bvh_t* tree, int leaf_id);

/**
 * @brief Returns number of leaves in the tree
 */
int bvh_leaf_count (const bvh_t* tree);

/**
 * @brief Depth-first walk over every node (internal + leaf).
 */
void  bvh_walk(const bvh_t* tree, bvh_walk_cb cb, void *ctx);

/**
 * @brief Total surface-area cost (lower = better balanced).
 */
float bvh_cost(const bvh_t* tree);

#endif /* PICO_BVH_H */

#ifdef PICO_BVH_IMPLEMENTATION

/*
 * Design notes
 * ────────────
 * The tree is a pool-allocated binary tree stored in a flat array.
 * Every node keeps:
 *   • A "padded" AABB (tight AABB padded by a user-supplied margin).
 *   • child[0], child[1] – BVH_NULL_ID for leaves.
 *   • parent             – BVH_NULL_ID for the root.
 *   • height             – 0 for leaves, max(child heights)+1 otherwise.
 *   • user_data          – only meaningful for leaves.
 *
 * Insertion (O(log n) expected)
 * ──────────────────────────────
 * We pick the best sibling for a new leaf using the surface-area heuristic:
 * the sibling that minimises the total induced cost increase walking back to
 * the root.  This is the exact O(log n) algorithm described by:
 *   Bittner et al. "Fast, Effective BVH Updates for Animated Scenes" (2015)
 * (also used by Box2D's b2DynamicTree).
 *
 * After every insertion / removal we walk back to the root and:
 *   1. Refit the ancestor AABBs.
 *   2. Apply one SAH rotation at each ancestor to reduce surface-area cost.
 *
 * Rotations (SAH balance)
 * ────────────────────────
 * At each internal node we consider swapping one of its grandchildren with
 * the other child.  If any swap reduces the node's induced surface area we
 * apply it.  This keeps the tree height near O(log n) without a full rebuild.
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

/* ── math primitives ──────────────────────────────────────────────────────────── */

static inline bvh_aabb_t bvh_aabb_pad(bvh_aabb_t a, float m)
{
    return (bvh_aabb_t){ {a.min.x-m, a.min.y-m}, {a.max.x+m, a.max.y+m} };
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

/* ── internal node ───────────────────────────────────────────────────────── */

typedef struct
{
    bvh_aabb_t  aabb;
    int         parent;
    int         child[2];   /* BVH_NULL_ID for leaves */
    int         height;     /* 0 = leaf */
    void       *user_data;  /* valid only for leaves */
    bool        allocated;
} bvh_node_t;

#define BVH_IS_LEAF(n)  ((n)->child[0] == BVH_NULL_ID)

/* ── tree structure ──────────────────────────────────────────────────────── */

#define BVH_INITIAL_CAPACITY 64

struct bvh_t
{
    bvh_node_t *nodes;
    int         capacity;
    int         root;
    int         free_list;   /* singly-linked free list via child[0] */
    int         leaf_count;
};

/* ── node pool ───────────────────────────────────────────────────────────── */

static void bvh_grow(bvh_t* t)
{
    int old_cap  = t->capacity;
    int new_cap  = old_cap * 2;
    t->nodes     = PICO_BVH_REALLOC(t->nodes, (size_t)new_cap * sizeof(bvh_node_t));

    PICO_BVH_ASSERT(t->nodes);

    memset(t->nodes + old_cap, 0, (size_t)(new_cap - old_cap) * sizeof(bvh_node_t));

    /* chain new slots onto the free list */
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
    bvh_node_t *n = &t->nodes[id];
    n->parent     = BVH_NULL_ID;
    n->child[0]   = BVH_NULL_ID;
    n->child[1]   = BVH_NULL_ID;
    n->height     = 0;
    n->user_data  = NULL;
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

/* ── helpers ─────────────────────────────────────────────────────────────── */

static void bvh_refit(bvh_t* t, int id)
{
    bvh_node_t *n   = &t->nodes[id];
    n->aabb     = bvh_aabb_union(t->nodes[n->child[0]].aabb,
                  t->nodes[n->child[1]].aabb);
    int h0      = t->nodes[n->child[0]].height;
    int h1      = t->nodes[n->child[1]].height;
    n->height   = 1 + (h0 > h1 ? h0 : h1);
}

/* ── SAH rotation ────────────────────────────────────────────────────────── */
/*
 * Consider all 4 possible swaps of grandchildren with the opposite child:
 *
 *      [A]              [A]
 *     /   \     →      /   \
 *   [B]   [C]        [B']  [C']
 *   / \   / \
 *  b0 b1 c0 c1
 *
 * We can swap (b0 <-> C), (b1 <-> C), (c0 <-> B), or (c1 <-> B).
 * Pick the swap that most reduces A's surface area.
 */
static void bvh_rotate(bvh_t* t, int a_id)
{
    bvh_node_t *A = &t->nodes[a_id];

    if (A->height < 2)
    {
        return;
    }

    int b_id = A->child[0];
    int c_id = A->child[1];
    bvh_node_t *B  = &t->nodes[b_id];
    bvh_node_t *C  = &t->nodes[c_id];

    float base_cost = bvh_aabb_perimeter(A->aabb);

    /* candidate costs: union of the node that stays with its new sibling */
    float costs[4] = { FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX };

    /* swap b0 <-> C  →  A.child[0]=C, B.child[0]=old-C, B.child[1]=b1 */
    if (!BVH_IS_LEAF(B))
    {
        costs[0] = bvh_aabb_perimeter(bvh_aabb_union(C->aabb,
                       t->nodes[B->child[1]].aabb));  /* new B cost */
        costs[1] = bvh_aabb_perimeter(bvh_aabb_union(C->aabb,
                       t->nodes[B->child[0]].aabb));  /* swap b1 <-> C */
    }
    /* swap c0 <-> B  →  A.child[1]=B, C.child[0]=old-B, C.child[1]=c1 */
    if (!BVH_IS_LEAF(C))
    {
        costs[2] = bvh_aabb_perimeter(bvh_aabb_union(B->aabb,
                       t->nodes[C->child[1]].aabb));
        costs[3] = bvh_aabb_perimeter(bvh_aabb_union(B->aabb,
                       t->nodes[C->child[0]].aabb));
    }

    /* find best candidate */
    int best = -1;
    float best_cost = base_cost;

    for (int i = 0; i < 4; ++i)
    {
        if (costs[i] < best_cost)
        {
            best_cost = costs[i]; best = i;
        }
    }
    if (best < 0)
    {
        return;  /* no improvement */
    }

    switch (best)
    {
    case 0:
    { /* swap B->child[0] <-> C */
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
    { /* swap B->child[1] <-> C */
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
    { /* swap C->child[0] <-> B */
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
    { /* swap C->child[1] <-> B */
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

/* Walk from `start` toward the root: bvh_refit + bvh_rotate each ancestor. */
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

/* ── best-sibling search (SAH) ───────────────────────────────────────────── */
/*
 * Branch-and-bound traversal to find the node that, when used as a sibling
 * for the new leaf L, minimises the total induced cost increase up to root.
 *
 * induced_cost(node) = cost(union(node, L)) - cost(node)
 *                    + Σ [cost(union(anc, L)) - cost(anc)] for each ancestor
 *
 * We maintain a priority queue (simple binary heap) keyed on a lower bound
 * of the induced cost to prune branches early.
 */
typedef struct
{
    int   id;
    float lower_bound;
} bvh_heap_entry_t;

typedef struct
{
    bvh_heap_entry_t *data;
    int               size;
    int               cap;
} bvh_min_heap_t;

static void bvh_heap_push(bvh_min_heap_t *h, bvh_heap_entry_t e)
{
    if (h->size == h->cap)
    {
        h->cap  = h->cap ? h->cap * 2 : 16;
        h->data = PICO_BVH_REALLOC(h->data, (size_t)h->cap * sizeof(bvh_heap_entry_t));

        PICO_BVH_ASSERT(h->data);
    }

    /* sift-up */
    int i = h->size++;
    h->data[i] = e;

    while (i > 0)
    {
        int p = (i - 1) / 2;

        if (h->data[p].lower_bound <= h->data[i].lower_bound)
        {
            break;
        }

        bvh_heap_entry_t tmp = h->data[p]; h->data[p] = h->data[i]; h->data[i] = tmp;
        i = p;
    }
}

static bvh_heap_entry_t bvh_heap_pop(bvh_min_heap_t *h)
{
    bvh_heap_entry_t top = h->data[0];
    h->data[0] = h->data[--h->size];

    /* sift-down */
    int i = 0;
    for (;;)
    {
        int l = 2*i+1, r = 2*i+2, smallest = i;
        if (l < h->size && h->data[l].lower_bound < h->data[smallest].lower_bound)
        {
            smallest = l;
        }

        if (r < h->size && h->data[r].lower_bound < h->data[smallest].lower_bound)
        {
            smallest = r;
        }

        if (smallest == i)
        {
            break;
        }

        bvh_heap_entry_t tmp = h->data[i]; h->data[i] = h->data[smallest];
        h->data[smallest] = tmp;
        i = smallest;
    }

    return top;
}

static int bvh_best_sibling(bvh_t* t, bvh_aabb_t L_aabb)
{
    float L_cost = bvh_aabb_perimeter(L_aabb);

    bvh_min_heap_t heap = {0};
    bvh_heap_push(&heap, (bvh_heap_entry_t){ t->root, 0.f });

    int   best_id   = t->root;
    float best_cost = FLT_MAX;

    while (heap.size > 0)
    {
        bvh_heap_entry_t e = bvh_heap_pop(&heap);

        if (e.lower_bound >= best_cost)
        {
            break;  /* prune */
        }

        bvh_node_t *node      = &t->nodes[e.id];
        bvh_aabb_t  combined  = bvh_aabb_union(node->aabb, L_aabb);
        float direct_cost = bvh_aabb_perimeter(combined);
        float total_cost  = direct_cost + e.lower_bound;

        if (total_cost < best_cost)
        {
            best_cost = total_cost;
            best_id   = e.id;
        }

        if (!BVH_IS_LEAF(node))
        {
            /* inherited cost that any descendant must pay */
            float inherited = e.lower_bound + direct_cost
                              - bvh_aabb_perimeter(node->aabb);

            /* lower-bound for children: assume they equal L (best case) */
            float lb = inherited + L_cost;

            if (lb < best_cost)
            {
                bvh_heap_push(&heap, (bvh_heap_entry_t){ node->child[0], inherited });
                bvh_heap_push(&heap, (bvh_heap_entry_t){ node->child[1], inherited });
            }
        }
    }

    PICO_BVH_FREE(heap.data);
    return best_id;
}

/* ── public: lifecycle ───────────────────────────────────────────────────── */

bvh_t *bvh_create(void)
{
    bvh_t* t = PICO_BVH_CALLOC(1, sizeof(bvh_t));
    PICO_BVH_ASSERT(t);

    t->capacity = BVH_INITIAL_CAPACITY;
    t->nodes    = PICO_BVH_CALLOC((size_t)t->capacity, sizeof(bvh_node_t));

    PICO_BVH_ASSERT(t->nodes);

    t->root       = BVH_NULL_ID;
    t->leaf_count = 0;

    /* build free list */
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

/* ── public: insert ──────────────────────────────────────────────────────── */

int bvh_insert(bvh_t* t, bvh_aabb_t aabb, float margin, void *user_data)
{
    int leaf_id      = bvh_alloc_node(t);
    bvh_node_t *leaf = &t->nodes[leaf_id];
    leaf->aabb       = margin > 0.f ? bvh_aabb_pad(aabb, margin) : aabb;
    leaf->user_data  = user_data;
    leaf->height    = 0;
    t->leaf_count++;

    /* empty tree */
    if (t->root == BVH_NULL_ID)
    {
        t->root = leaf_id;
        leaf->parent = BVH_NULL_ID;
        return leaf_id;
    }

    /* find best sibling */
    int sib_id = bvh_best_sibling(t, leaf->aabb);

    /* create a new internal node to replace sibling */
    int new_id           = bvh_alloc_node(t);
    bvh_node_t *new_node = &t->nodes[new_id];
    int old_parent = t->nodes[sib_id].parent;

    new_node->parent   = old_parent;
    new_node->child[0] = sib_id;
    new_node->child[1] = leaf_id;
    new_node->height   = 0;   /* will be set by bvh_refit */

    t->nodes[sib_id].parent  = new_id;
    t->nodes[leaf_id].parent = new_id;

    if (old_parent == BVH_NULL_ID)
    {
        t->root = new_id;
    }
    else
    {
        bvh_node_t *op = &t->nodes[old_parent];

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

/* ── public: remove ──────────────────────────────────────────────────────── */

void bvh_remove(bvh_t* t, int leaf_id)
{
    PICO_BVH_ASSERT(leaf_id >= 0 && leaf_id < t->capacity);
    PICO_BVH_ASSERT(BVH_IS_LEAF(&t->nodes[leaf_id]));

    int parent_id = t->nodes[leaf_id].parent;
    bvh_free_node(t, leaf_id);
    t->leaf_count--;

    if (parent_id == BVH_NULL_ID)
    {
        /* was the only node */
        t->root = BVH_NULL_ID;
        return;
    }

    /* find the sibling, pull it up to replace the parent */
    bvh_node_t *parent = &t->nodes[parent_id];
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
        bvh_node_t *gp = &t->nodes[grandparent];

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

/* ── public: move ────────────────────────────────────────────────────────── */

bool bvh_move(bvh_t* t, int leaf_id, bvh_aabb_t new_aabb, float margin)
{
    PICO_BVH_ASSERT(BVH_IS_LEAF(&t->nodes[leaf_id]));

    bvh_aabb_t padded = margin > 0.f ? bvh_aabb_pad(new_aabb, margin) : new_aabb;

    /* no restructuring needed if the padded AABB still contains the new one */
    if (bvh_aabb_contains(t->nodes[leaf_id].aabb, new_aabb))
    {
        /* optionally shrink if the padded box is much bigger than needed */
        bvh_aabb_t big = bvh_aabb_pad(new_aabb, margin * 4.f);

        if (bvh_aabb_contains(big, t->nodes[leaf_id].aabb))
        {
            return false;
        }
    }

    void *ud = t->nodes[leaf_id].user_data;
    bvh_remove(t, leaf_id);

    /* bvh_remove freed leaf_id (and its parent).  Ensure leaf_id sits at the
     * front of the free list so bvh_alloc_node returns it first, preserving
     * the caller's handle. */
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

/* ── public: query (AABB) ────────────────────────────────────────────────── */

/* Iterative DFS using an explicit stack to avoid recursion overhead. */
void bvh_query_aabb(const bvh_t* t, bvh_aabb_t query, bvh_query_cb cb, void *ctx)
{
    if (t->root == BVH_NULL_ID)
    {
        return;
    }

    /* stack: fixed-size.  2*height+2 suffices for a balanced tree.
     * We allocate generously; could also be dynamic. */
    int stack[1024];
    int top = 0;
    stack[top++] = t->root;

    while (top > 0)
    {
        int id = stack[--top];
        if (id == BVH_NULL_ID)
        {
            continue;
        }

        const bvh_node_t *n = &t->nodes[id];

        if (!bvh_aabb_overlaps(n->aabb, query))
        {
            continue;
        }

        if (BVH_IS_LEAF(n))
        {
            if (!cb(id, n->user_data, ctx))
            {
                return;
            }
        }
        else
        {
            PICO_BVH_ASSERT(top + 2 <= 1024);
            stack[top++] = n->child[0];
            stack[top++] = n->child[1];
        }
    }
}

/* ── public: query (ray) ─────────────────────────────────────────────────── */
/*
 * Slab test for ray vs AABB intersection.
 * Returns true if the ray hits the AABB within [0, max_t].
 */
static bool bvh_ray_aabb(bvh_vec2_t origin, bvh_vec2_t inv_dir, bvh_aabb_t aabb, float max_t)
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

    return tmax >= 0.f && tmin <= tmax && tmin <= max_t;
}

#define PICO_BVH_HUGE 1e-9f

void bvh_query_ray(const bvh_t* t,
                   bvh_vec2_t origin, bvh_vec2_t dir, float max_t,
                   bvh_query_cb cb, void *ctx)
{
    if (t->root == BVH_NULL_ID)
    {
        return;
    }

    /* precompute reciprocal direction (handle zero-components) */
    bvh_vec2_t inv_dir =
    {
        fabsf(dir.x) > PICO_BVH_HUGE ? 1.f / dir.x : (dir.x >= 0.f ? FLT_MAX : -FLT_MAX),
        fabsf(dir.y) > PICO_BVH_HUGE ? 1.f / dir.y : (dir.y >= 0.f ? FLT_MAX : -FLT_MAX)
    };

    int stack[1024]; // TODO: define constant
    int top = 0;
    stack[top++] = t->root;

    while (top > 0)
    {
        int id = stack[--top];
        if (id == BVH_NULL_ID)
        {
            continue;
        }

        const bvh_node_t *n = &t->nodes[id];
        if (!bvh_ray_aabb(origin, inv_dir, n->aabb, max_t))
        {
            continue;
        }

        if (BVH_IS_LEAF(n))
        {
            if (!cb(id, n->user_data, ctx))
            {
                return;
            }
        }
        else
        {
            PICO_BVH_ASSERT(top + 2 <= 1024);
            stack[top++] = n->child[0];
            stack[top++] = n->child[1];
        }
    }
}

/* ── public: accessors ───────────────────────────────────────────────────── */

void *bvh_user_data(const bvh_t* t, int leaf_id)
{
    PICO_BVH_ASSERT(BVH_IS_LEAF(&t->nodes[leaf_id]));
    return t->nodes[leaf_id].user_data;
}

bvh_aabb_t bvh_padded_aabb(const bvh_t* t, int leaf_id)
{
    return t->nodes[leaf_id].aabb;
}

int bvh_leaf_count(const bvh_t* t) { return t->leaf_count; }

/* ── public: walk ────────────────────────────────────────────────────────── */

static void bvh_walk_rec(const bvh_t* t, int id, int depth, bvh_walk_cb cb, void *ctx)
{
    if (id == BVH_NULL_ID)
    {
        return;
    }

    const bvh_node_t *n = &t->nodes[id];
    cb(n->aabb, depth, BVH_IS_LEAF(n), n->user_data, ctx);

    if (!BVH_IS_LEAF(n))
    {
        bvh_walk_rec(t, n->child[0], depth + 1, cb, ctx);
        bvh_walk_rec(t, n->child[1], depth + 1, cb, ctx);
    }
}

void bvh_walk(const bvh_t* t, bvh_walk_cb cb, void *ctx)
{
    bvh_walk_rec(t, t->root, 0, cb, ctx);
}

/* ── public: cost ────────────────────────────────────────────────────────── */

static float bvh_cost_rec(const bvh_t* t, int id)
{
    if (id == BVH_NULL_ID)
    {
        return 0.f;
    }

    const bvh_node_t *n = &t->nodes[id];
    float c = bvh_aabb_perimeter(n->aabb);

    if (!BVH_IS_LEAF(n))
    {
        c += bvh_cost_rec(t, n->child[0]) + bvh_cost_rec(t, n->child[1]);
    }
    return c;
}

float bvh_cost(const bvh_t* t) { return bvh_cost_rec(t, t->root); }

#endif // PICO_BVH_IMPLEMENTATION
