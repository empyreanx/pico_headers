#define PICO_BVH_UDATA_TYPE void*
#define PICO_BVH_IMPLEMENTATION
#include "../pico_bvh.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

#include <math.h>

typedef struct
{
    int ids[256];
    int count;
    int stop_after;
} test_hits_t;

typedef struct
{
    int total_nodes;
    int leaf_nodes;
    int internal_nodes;
    int max_depth;
    int first_depth;
    bool first_is_leaf;
    bool saw_first;
} walk_stats_t;

static bvh_aabb_t test_make_aabb(float min_x, float min_y, float max_x, float max_y)
{
    return (bvh_aabb_t){ { min_x, min_y }, { max_x, max_y } };
}

static bool test_float_equal(float a, float b)
{
    return fabsf(a - b) < 1e-6f;
}

static bool test_aabb_equal(bvh_aabb_t a, bvh_aabb_t b)
{
    return test_float_equal(a.min.x, b.min.x)
        && test_float_equal(a.min.y, b.min.y)
        && test_float_equal(a.max.x, b.max.x)
        && test_float_equal(a.max.y, b.max.y);
}

static bool test_collect_hits(int leaf_id, bvh_udata_t user_data, void *ctx)
{
    test_hits_t *hits = ctx;
    (void)user_data;

    hits->ids[hits->count++] = leaf_id;
    return hits->stop_after <= 0 || hits->count < hits->stop_after;
}

static bool test_hits_contains(const test_hits_t *hits, int leaf_id)
{
    for (int i = 0; i < hits->count; ++i)
    {
        if (hits->ids[i] == leaf_id)
        {
            return true;
        }
    }
    return false;
}

static void test_record_walk(bvh_aabb_t aabb, int depth, bool is_leaf,
                             bvh_udata_t user_data, void *ctx)
{
    walk_stats_t *stats = ctx;
    (void)aabb;
    (void)user_data;

    if (!stats->saw_first)
    {
        stats->first_depth = depth;
        stats->first_is_leaf = is_leaf;
        stats->saw_first = true;
    }

    stats->total_nodes++;
    if (is_leaf)
    {
        stats->leaf_nodes++;
    }
    else
    {
        stats->internal_nodes++;
    }

    if (depth > stats->max_depth)
    {
        stats->max_depth = depth;
    }
}

TEST_CASE(test_insert_and_accessors)
{
    bvh_t *tree = bvh_create();
    int payload_a = 11;
    int payload_b = 22;
    bvh_aabb_t aabb_a = test_make_aabb(0.f, 0.f, 1.f, 1.f);
    bvh_aabb_t aabb_b = test_make_aabb(3.f, 4.f, 5.f, 6.f);
    int id_a;
    int id_b;
    test_hits_t hits = {0};

    REQUIRE(tree != NULL);
    REQUIRE(bvh_get_leaf_count(tree) == 0);

    id_a = bvh_insert(tree, aabb_a, 0.5f, &payload_a);
    id_b = bvh_insert(tree, aabb_b, 0.f, &payload_b);

    REQUIRE(id_a != id_b);
    REQUIRE(bvh_get_leaf_count(tree) == 2);
    REQUIRE(bvh_get_user_data(tree, id_a) == &payload_a);
    REQUIRE(bvh_get_user_data(tree, id_b) == &payload_b);
    REQUIRE(test_aabb_equal(bvh_get_padded_aabb(tree, id_a),
                            test_make_aabb(-0.5f, -0.5f, 1.5f, 1.5f)));
    REQUIRE(test_aabb_equal(bvh_get_padded_aabb(tree, id_b), aabb_b));

    bvh_query_aabb(tree, test_make_aabb(-1.f, -1.f, 10.f, 10.f), test_collect_hits, &hits);
    REQUIRE(hits.count == 2);
    REQUIRE(test_hits_contains(&hits, id_a));
    REQUIRE(test_hits_contains(&hits, id_b));

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_query_aabb_early_exit)
{
    bvh_t *tree = bvh_create();
    test_hits_t hits = {0};
    test_hits_t empty_hits = {0};

    bvh_insert(tree, test_make_aabb(0.f, 0.f, 1.f, 1.f), 0.f, 0);
    bvh_insert(tree, test_make_aabb(2.f, 2.f, 3.f, 3.f), 0.f, 0);
    bvh_insert(tree, test_make_aabb(10.f, 10.f, 11.f, 11.f), 0.f, 0);

    hits.stop_after = 1;
    bvh_query_aabb(tree, test_make_aabb(-1.f, -1.f, 5.f, 5.f), test_collect_hits, &hits);
    REQUIRE(hits.count == 1);

    bvh_query_aabb(tree, test_make_aabb(20.f, 20.f, 21.f, 21.f), test_collect_hits, &empty_hits);
    REQUIRE(empty_hits.count == 0);

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_query_ray)
{
    bvh_t *tree = bvh_create();
    int id_a = bvh_insert(tree, test_make_aabb(2.f, 0.f, 3.f, 1.f), 0.f, 0);
    int id_b = bvh_insert(tree, test_make_aabb(6.f, 0.f, 7.f, 1.f), 0.f, 0);
    int id_c = bvh_insert(tree, test_make_aabb(2.f, 3.f, 3.f, 4.f), 0.f, 0);
    test_hits_t hits = {0};
    test_hits_t short_hits = {0};

    bvh_query_ray(tree,
                  (bvh_vec2_t){ 0.f, 0.5f },
                  (bvh_vec2_t){ 1.f, 0.f },
                  10.f,
                  test_collect_hits,
                  &hits);

    REQUIRE(hits.count == 2);
    REQUIRE(test_hits_contains(&hits, id_a));
    REQUIRE(test_hits_contains(&hits, id_b));
    REQUIRE(!test_hits_contains(&hits, id_c));

    bvh_query_ray(tree,
                  (bvh_vec2_t){ 0.f, 0.5f },
                  (bvh_vec2_t){ 1.f, 0.f },
                  2.5f,
                  test_collect_hits,
                  &short_hits);

    REQUIRE(short_hits.count == 1);
    REQUIRE(test_hits_contains(&short_hits, id_a));

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_move_reuses_id_and_updates_bounds)
{
    bvh_t *tree = bvh_create();
    int payload = 7;
    int leaf_id = bvh_insert(tree, test_make_aabb(0.f, 0.f, 1.f, 1.f), 1.f, &payload);
    test_hits_t old_hits = {0};
    test_hits_t new_hits = {0};
    bool moved;

    moved = bvh_move(tree, leaf_id, test_make_aabb(0.25f, 0.25f, 1.25f, 1.25f), 1.f);
    REQUIRE(!moved);
    REQUIRE(test_aabb_equal(bvh_get_padded_aabb(tree, leaf_id),
                            test_make_aabb(-1.f, -1.f, 2.f, 2.f)));

    moved = bvh_move(tree, leaf_id, test_make_aabb(10.f, 10.f, 11.f, 11.f), 1.f);
    REQUIRE(moved);
    REQUIRE(bvh_get_user_data(tree, leaf_id) == &payload);
    REQUIRE(test_aabb_equal(bvh_get_padded_aabb(tree, leaf_id),
                            test_make_aabb(9.f, 9.f, 12.f, 12.f)));

    bvh_query_aabb(tree, test_make_aabb(-2.f, -2.f, 2.f, 2.f), test_collect_hits, &old_hits);
    bvh_query_aabb(tree, test_make_aabb(8.f, 8.f, 13.f, 13.f), test_collect_hits, &new_hits);

    REQUIRE(old_hits.count == 0);
    REQUIRE(new_hits.count == 1);
    REQUIRE(new_hits.ids[0] == leaf_id);

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_remove_walk_and_cost)
{
    bvh_t *tree = bvh_create();
    int id_a = bvh_insert(tree, test_make_aabb(0.f, 0.f, 1.f, 1.f), 0.f, 0);
    int id_b = bvh_insert(tree, test_make_aabb(4.f, 0.f, 5.f, 1.f), 0.f, 0);
    int id_c = bvh_insert(tree, test_make_aabb(8.f, 0.f, 9.f, 1.f), 0.f, 0);
    walk_stats_t stats = {0};
    test_hits_t hits = {0};
    float cost;

    bvh_remove(tree, id_b);

    REQUIRE(bvh_get_leaf_count(tree) == 2);

    bvh_walk(tree, test_record_walk, &stats);
    REQUIRE(stats.saw_first);
    REQUIRE(stats.first_depth == 0);
    REQUIRE(!stats.first_is_leaf);
    REQUIRE(stats.total_nodes == 3);
    REQUIRE(stats.leaf_nodes == 2);
    REQUIRE(stats.internal_nodes == 1);
    REQUIRE(stats.max_depth == 1);

    bvh_query_aabb(tree, test_make_aabb(-1.f, -1.f, 10.f, 2.f), test_collect_hits, &hits);
    REQUIRE(hits.count == 2);
    REQUIRE(test_hits_contains(&hits, id_a));
    REQUIRE(!test_hits_contains(&hits, id_b));
    REQUIRE(test_hits_contains(&hits, id_c));

    cost = bvh_get_cost(tree);
    REQUIRE(test_float_equal(cost, 28.f));

    bvh_destroy(tree);
    return true;
}

/* ── additional tests ─────────────────────────────────────────────────── */

TEST_CASE(test_empty_tree_queries)
{
    bvh_t *tree = bvh_create();
    test_hits_t hits = {0};

    REQUIRE(bvh_get_leaf_count(tree) == 0);
    REQUIRE(test_float_equal(bvh_get_cost(tree), 0.f));

    /* AABB query on empty tree should be a no-op */
    bvh_query_aabb(tree, test_make_aabb(-10.f, -10.f, 10.f, 10.f),
                   test_collect_hits, &hits);
    REQUIRE(hits.count == 0);

    /* ray query on empty tree should be a no-op */
    bvh_query_ray(tree, (bvh_vec2_t){0.f, 0.f}, (bvh_vec2_t){1.f, 0.f},
                  100.f, test_collect_hits, &hits);
    REQUIRE(hits.count == 0);

    /* walk on empty tree */
    walk_stats_t ws = {0};
    bvh_walk(tree, test_record_walk, &ws);
    REQUIRE(!ws.saw_first);
    REQUIRE(ws.total_nodes == 0);

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_destroy_null)
{
    /* bvh_destroy(NULL) must not crash */
    bvh_destroy(NULL);
    return true;
}

TEST_CASE(test_single_leaf)
{
    bvh_t *tree = bvh_create();
    int payload = 42;
    bvh_aabb_t aabb = test_make_aabb(1.f, 2.f, 3.f, 4.f);
    int id = bvh_insert(tree, aabb, 0.f, &payload);

    REQUIRE(bvh_get_leaf_count(tree) == 1);
    REQUIRE(bvh_get_user_data(tree, id) == &payload);
    REQUIRE(test_aabb_equal(bvh_get_padded_aabb(tree, id), aabb));

    /* walk on single leaf: root is the leaf */
    walk_stats_t ws = {0};
    bvh_walk(tree, test_record_walk, &ws);
    REQUIRE(ws.total_nodes == 1);
    REQUIRE(ws.leaf_nodes == 1);
    REQUIRE(ws.internal_nodes == 0);
    REQUIRE(ws.first_depth == 0);
    REQUIRE(ws.first_is_leaf);

    /* cost on single leaf */
    float c = bvh_get_cost(tree);
    float expected_cost = 2.f * ((3.f - 1.f) + (4.f - 2.f));
    REQUIRE(test_float_equal(c, expected_cost));

    /* AABB query hits */
    test_hits_t hits = {0};
    bvh_query_aabb(tree, test_make_aabb(0.f, 0.f, 5.f, 5.f),
                   test_collect_hits, &hits);
    REQUIRE(hits.count == 1);
    REQUIRE(hits.ids[0] == id);

    /* remove the only leaf */
    bvh_remove(tree, id);
    REQUIRE(bvh_get_leaf_count(tree) == 0);

    /* queries on now-empty tree */
    test_hits_t h2 = {0};
    bvh_query_aabb(tree, test_make_aabb(-100.f, -100.f, 100.f, 100.f),
                   test_collect_hits, &h2);
    REQUIRE(h2.count == 0);

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_remove_all_leaves)
{
    bvh_t *tree = bvh_create();
    int ids[4];
    int i;

    for (i = 0; i < 4; ++i)
    {
        ids[i] = bvh_insert(tree, test_make_aabb((float)i * 3.f, 0.f,
                                                  (float)i * 3.f + 1.f, 1.f),
                             0.f, 0);
    }
    REQUIRE(bvh_get_leaf_count(tree) == 4);

    for (i = 0; i < 4; ++i)
    {
        bvh_remove(tree, ids[i]);
    }
    REQUIRE(bvh_get_leaf_count(tree) == 0);

    /* the tree is reusable after removing everything */
    int new_id = bvh_insert(tree, test_make_aabb(0.f, 0.f, 1.f, 1.f), 0.f, 0);
    REQUIRE(bvh_get_leaf_count(tree) == 1);

    test_hits_t hits = {0};
    bvh_query_aabb(tree, test_make_aabb(-1.f, -1.f, 2.f, 2.f),
                   test_collect_hits, &hits);
    REQUIRE(hits.count == 1);
    REQUIRE(hits.ids[0] == new_id);

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_overlapping_aabbs)
{
    bvh_t *tree = bvh_create();
    int id_a = bvh_insert(tree, test_make_aabb(0.f, 0.f, 5.f, 5.f), 0.f, 0);
    int id_b = bvh_insert(tree, test_make_aabb(3.f, 3.f, 8.f, 8.f), 0.f, 0);
    int id_c = bvh_insert(tree, test_make_aabb(4.f, 4.f, 6.f, 6.f), 0.f, 0);

    /* a query in the overlap region should find all three */
    test_hits_t hits = {0};
    bvh_query_aabb(tree, test_make_aabb(4.f, 4.f, 4.5f, 4.5f),
                   test_collect_hits, &hits);
    REQUIRE(hits.count == 3);
    REQUIRE(test_hits_contains(&hits, id_a));
    REQUIRE(test_hits_contains(&hits, id_b));
    REQUIRE(test_hits_contains(&hits, id_c));

    /* a query that only touches one */
    test_hits_t h2 = {0};
    bvh_query_aabb(tree, test_make_aabb(0.f, 0.f, 0.5f, 0.5f),
                   test_collect_hits, &h2);
    REQUIRE(h2.count == 1);
    REQUIRE(h2.ids[0] == id_a);

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_ray_diagonal)
{
    bvh_t *tree = bvh_create();
    int id_a = bvh_insert(tree, test_make_aabb(2.f, 2.f, 3.f, 3.f), 0.f, 0);
    int id_b = bvh_insert(tree, test_make_aabb(5.f, 5.f, 6.f, 6.f), 0.f, 0);
    int id_c = bvh_insert(tree, test_make_aabb(0.f, 5.f, 1.f, 6.f), 0.f, 0);

    /* diagonal ray from origin going (1,1) should hit a and b but not c */
    test_hits_t hits = {0};
    bvh_query_ray(tree, (bvh_vec2_t){0.f, 0.f}, (bvh_vec2_t){1.f, 1.f},
                  10.f, test_collect_hits, &hits);
    REQUIRE(test_hits_contains(&hits, id_a));
    REQUIRE(test_hits_contains(&hits, id_b));
    REQUIRE(!test_hits_contains(&hits, id_c));

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_ray_miss)
{
    bvh_t *tree = bvh_create();
    bvh_insert(tree, test_make_aabb(5.f, 5.f, 6.f, 6.f), 0.f, 0);

    /* ray going in the wrong direction */
    test_hits_t h1 = {0};
    bvh_query_ray(tree, (bvh_vec2_t){0.f, 0.f}, (bvh_vec2_t){-1.f, 0.f},
                  100.f, test_collect_hits, &h1);
    REQUIRE(h1.count == 0);

    /* ray too short to reach the box */
    test_hits_t h2 = {0};
    bvh_query_ray(tree, (bvh_vec2_t){0.f, 5.5f}, (bvh_vec2_t){1.f, 0.f},
                  2.f, test_collect_hits, &h2);
    REQUIRE(h2.count == 0);

    /* ray parallel but offset */
    test_hits_t h3 = {0};
    bvh_query_ray(tree, (bvh_vec2_t){0.f, 10.f}, (bvh_vec2_t){1.f, 0.f},
                  100.f, test_collect_hits, &h3);
    REQUIRE(h3.count == 0);

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_ray_early_exit)
{
    bvh_t *tree = bvh_create();
    bvh_insert(tree, test_make_aabb(2.f, -0.5f, 3.f,  0.5f), 0.f, 0);
    bvh_insert(tree, test_make_aabb(5.f, -0.5f, 6.f,  0.5f), 0.f, 0);
    bvh_insert(tree, test_make_aabb(8.f, -0.5f, 9.f,  0.5f), 0.f, 0);

    test_hits_t hits = {0};
    hits.stop_after = 1;
    bvh_query_ray(tree, (bvh_vec2_t){0.f, 0.f}, (bvh_vec2_t){1.f, 0.f},
                  20.f, test_collect_hits, &hits);
    REQUIRE(hits.count == 1);

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_ray_vertical)
{
    bvh_t *tree = bvh_create();
    int id = bvh_insert(tree, test_make_aabb(-1.f, 3.f, 1.f, 5.f), 0.f, 0);

    /* vertical ray (dir.x == 0) exercises the near-zero reciprocal path */
    test_hits_t hits = {0};
    bvh_query_ray(tree, (bvh_vec2_t){0.f, 0.f}, (bvh_vec2_t){0.f, 1.f},
                  10.f, test_collect_hits, &hits);
    REQUIRE(hits.count == 1);
    REQUIRE(hits.ids[0] == id);

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_move_among_multiple)
{
    bvh_t *tree = bvh_create();
    int payload_a = 1, payload_b = 2, payload_c = 3;
    int id_a = bvh_insert(tree, test_make_aabb(0.f, 0.f, 1.f, 1.f), 0.f, &payload_a);
    int id_b = bvh_insert(tree, test_make_aabb(5.f, 0.f, 6.f, 1.f), 0.f, &payload_b);
    int id_c = bvh_insert(tree, test_make_aabb(10.f, 0.f, 11.f, 1.f), 0.f, &payload_c);

    /* move b far away */
    bvh_move(tree, id_b, test_make_aabb(50.f, 50.f, 51.f, 51.f), 0.f);
    REQUIRE(bvh_get_leaf_count(tree) == 3);

    /* a and c unchanged */
    REQUIRE(bvh_get_user_data(tree, id_a) == &payload_a);
    REQUIRE(bvh_get_user_data(tree, id_c) == &payload_c);

    /* b's user_data preserved */
    REQUIRE(bvh_get_user_data(tree, id_b) == &payload_b);

    /* query the old region: only a and c */
    test_hits_t h1 = {0};
    bvh_query_aabb(tree, test_make_aabb(-1.f, -1.f, 12.f, 2.f),
                   test_collect_hits, &h1);
    REQUIRE(h1.count == 2);
    REQUIRE(test_hits_contains(&h1, id_a));
    REQUIRE(test_hits_contains(&h1, id_c));
    REQUIRE(!test_hits_contains(&h1, id_b));

    /* query b's new region */
    test_hits_t h2 = {0};
    bvh_query_aabb(tree, test_make_aabb(49.f, 49.f, 52.f, 52.f),
                   test_collect_hits, &h2);
    REQUIRE(h2.count == 1);
    REQUIRE(h2.ids[0] == id_b);

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_insert_remove_reinsert)
{
    bvh_t *tree = bvh_create();
    int id1 = bvh_insert(tree, test_make_aabb(0.f, 0.f, 1.f, 1.f), 0.f, 0);
    int id2 = bvh_insert(tree, test_make_aabb(2.f, 0.f, 3.f, 1.f), 0.f, 0);

    bvh_remove(tree, id1);
    bvh_remove(tree, id2);
    REQUIRE(bvh_get_leaf_count(tree) == 0);

    /* reinsert – IDs should come from the free list */
    int id3 = bvh_insert(tree, test_make_aabb(0.f, 0.f, 1.f, 1.f), 0.f, 0);
    int id4 = bvh_insert(tree, test_make_aabb(2.f, 0.f, 3.f, 1.f), 0.f, 0);
    REQUIRE(bvh_get_leaf_count(tree) == 2);

    test_hits_t hits = {0};
    bvh_query_aabb(tree, test_make_aabb(-1.f, -1.f, 4.f, 2.f),
                   test_collect_hits, &hits);
    REQUIRE(hits.count == 2);
    REQUIRE(test_hits_contains(&hits, id3));
    REQUIRE(test_hits_contains(&hits, id4));

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_pool_growth)
{
    /* BVH_INITIAL_CAPACITY is 64; inserting 100 leaves forces bvh_grow */
    bvh_t *tree = bvh_create();
    int ids[100];
    int i;

    for (i = 0; i < 100; ++i)
    {
        float x = (float)(i % 10) * 3.f;
        float y = (float)(i / 10) * 3.f;

        ids[i] = bvh_insert(tree, test_make_aabb(x, y, x + 1.f, y + 1.f),
                            0.f, 0);
    }
    REQUIRE(bvh_get_leaf_count(tree) == 100);

    /* verify every leaf is queryable */
    test_hits_t hits = {0};
    bvh_query_aabb(tree, test_make_aabb(-1.f, -1.f, 100.f, 100.f),
                   test_collect_hits, &hits);
    REQUIRE(hits.count == 100);

    for (i = 0; i < 100; ++i)
    {
        REQUIRE(test_hits_contains(&hits, ids[i]));
    }

    /* verify user_data round-trip for a sample */
    int tag = 999;
    int tagged_id = bvh_insert(tree, test_make_aabb(50.f, 50.f, 51.f, 51.f),
                               0.f, &tag);
    REQUIRE(bvh_get_user_data(tree, tagged_id) == &tag);

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_cost_decreases_or_stable_after_removal)
{
    bvh_t *tree = bvh_create();
    int i;

    for (i = 0; i < 10; ++i)
    {
        bvh_insert(tree, test_make_aabb((float)i * 2.f, 0.f,
                                        (float)i * 2.f + 1.f, 1.f),
                   0.f, 0);
    }

    float cost_before = bvh_get_cost(tree);
    REQUIRE(cost_before > 0.f);

    /* remove the last 5 leaves (from the center outward) */
    /* We can only remove by id.  Insert returns sequential IDs, so
     * the first 10 leaf IDs are deterministic.  We stored none, but
     * we can query them. */
    test_hits_t hits = {0};
    bvh_query_aabb(tree, test_make_aabb(9.f, -1.f, 20.f, 2.f),
                   test_collect_hits, &hits);

    for (i = 0; i < hits.count; ++i)
    {
        bvh_remove(tree, hits.ids[i]);
    }

    float cost_after = bvh_get_cost(tree);
    REQUIRE(cost_after < cost_before);

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_walk_user_data)
{
    bvh_t *tree = bvh_create();
    int vals[3] = {10, 20, 30};
    bvh_insert(tree, test_make_aabb(0.f, 0.f, 1.f, 1.f), 0.f, &vals[0]);
    bvh_insert(tree, test_make_aabb(3.f, 0.f, 4.f, 1.f), 0.f, &vals[1]);
    bvh_insert(tree, test_make_aabb(6.f, 0.f, 7.f, 1.f), 0.f, &vals[2]);

    /* query all leaves and verify user_data via bvh_user_data accessor */
    test_hits_t hits = {0};
    bvh_query_aabb(tree, test_make_aabb(-1.f, -1.f, 10.f, 2.f),
                   test_collect_hits, &hits);

    bool found[3] = {false, false, false};
    for (int i = 0; i < hits.count; ++i)
    {
        bvh_udata_t ud = bvh_get_user_data(tree, hits.ids[i]);
        for (int j = 0; j < 3; ++j)
        {
            if (ud == &vals[j])
            {
                found[j] = true;
            }
        }
    }
    REQUIRE(found[0] && found[1] && found[2]);

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_query_touching_edges)
{
    /* AABBs that share an edge should overlap per the <= comparisons */
    bvh_t *tree = bvh_create();
    int id_a = bvh_insert(tree, test_make_aabb(0.f, 0.f, 1.f, 1.f), 0.f, 0);
    int id_b = bvh_insert(tree, test_make_aabb(1.f, 0.f, 2.f, 1.f), 0.f, 0);

    /* query exactly at the shared edge */
    test_hits_t hits = {0};
    bvh_query_aabb(tree, test_make_aabb(1.f, 0.f, 1.f, 1.f),
                   test_collect_hits, &hits);
    REQUIRE(hits.count == 2);
    REQUIRE(test_hits_contains(&hits, id_a));
    REQUIRE(test_hits_contains(&hits, id_b));

    bvh_destroy(tree);
    return true;
}

TEST_CASE(test_fat_aabb_margin)
{
    bvh_t *tree = bvh_create();
    bvh_aabb_t tight = test_make_aabb(5.f, 5.f, 7.f, 7.f);
    float margin = 2.f;
    int id = bvh_insert(tree, tight, margin, 0);

    bvh_aabb_t fat = bvh_get_padded_aabb(tree, id);
    REQUIRE(test_aabb_equal(fat, test_make_aabb(3.f, 3.f, 9.f, 9.f)));

    /* the fat AABB should be hittable even outside the tight box */
    test_hits_t hits = {0};
    bvh_query_aabb(tree, test_make_aabb(3.5f, 3.5f, 4.f, 4.f),
                   test_collect_hits, &hits);
    REQUIRE(hits.count == 1);
    REQUIRE(hits.ids[0] == id);

    bvh_destroy(tree);
    return true;
}

TEST_SUITE(suite_bvh_cb)
{
    RUN_TEST_CASE(test_insert_and_accessors);
    RUN_TEST_CASE(test_query_aabb_early_exit);
    RUN_TEST_CASE(test_query_ray);
    RUN_TEST_CASE(test_move_reuses_id_and_updates_bounds);
    RUN_TEST_CASE(test_remove_walk_and_cost);
    RUN_TEST_CASE(test_empty_tree_queries);
    RUN_TEST_CASE(test_destroy_null);
    RUN_TEST_CASE(test_single_leaf);
    RUN_TEST_CASE(test_remove_all_leaves);
    RUN_TEST_CASE(test_overlapping_aabbs);
    RUN_TEST_CASE(test_ray_diagonal);
    RUN_TEST_CASE(test_ray_miss);
    RUN_TEST_CASE(test_ray_early_exit);
    RUN_TEST_CASE(test_ray_vertical);
    RUN_TEST_CASE(test_move_among_multiple);
    RUN_TEST_CASE(test_insert_remove_reinsert);
    RUN_TEST_CASE(test_pool_growth);
    RUN_TEST_CASE(test_cost_decreases_or_stable_after_removal);
    RUN_TEST_CASE(test_walk_user_data);
    RUN_TEST_CASE(test_query_touching_edges);
    RUN_TEST_CASE(test_fat_aabb_margin);
}

int main()
{
    pu_display_colors(true);
    RUN_TEST_SUITE(suite_bvh_cb);
    pu_print_stats();
    return pu_test_failed();
}
