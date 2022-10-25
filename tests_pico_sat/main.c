#include <stdio.h>

#define PICO_SAT_IMPLEMENTATION
#include "../pico_sat.h"

#define PICO_MATH_IMPLEMENTATION
#include "../pico_math.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

PU_TEST(test_aabb_aabb_collide)
{
    pm_b2 aabb1 = pm_b2_make(5, 5, 2, 2);
    sat_poly_t p1 = sat_aabb_to_poly(&aabb1);

    // Collide right side
    {
        pm_b2 aabb2 = pm_b2_make(6, 6, 2, 2);
        sat_poly_t p2 = sat_aabb_to_poly(&aabb2);

        sat_manifold_t manifold;

        PU_ASSERT(sat_test_poly_poly(&p1, &p2, &manifold));

        PU_ASSERT(pm_equal(manifold.overlap, 1));
        PU_ASSERT(pm_v2_equal(manifold.normal, pm_v2_make(1, 0)));
        PU_ASSERT(pm_v2_equal(manifold.vector, pm_v2_make(1, 0)));
    }

    // Collide left side
    {
        pm_b2 aabb2 = pm_b2_make(4, 5, 2, 2);
        sat_poly_t p2 = sat_aabb_to_poly(&aabb2);

        sat_manifold_t manifold;

        PU_ASSERT(sat_test_poly_poly(&p1, &p2, &manifold));

        PU_ASSERT(pm_equal(manifold.overlap, 1));
        PU_ASSERT(pm_v2_equal(manifold.normal, pm_v2_make(-1, 0)));
        PU_ASSERT(pm_v2_equal(manifold.vector, pm_v2_make(-1, 0)));
    }

    return true;
}

PU_TEST(test_aabb_aabb_not_collide)
{
    pm_b2 aabb1 = pm_b2_make(5, 5, 2, 2);
    sat_poly_t p1 = sat_aabb_to_poly(&aabb1);

    // Diagonal
    {
        pm_b2 aabb2 = pm_b2_make(9, 9, 2, 2);
        sat_poly_t p2 = sat_aabb_to_poly(&aabb2);

        PU_ASSERT(!sat_test_poly_poly(&p1, &p2, NULL));
    }

    // Left
    {
        pm_b2 aabb2 = pm_b2_make(3, 5, 2, 2);
        sat_poly_t p2 = sat_aabb_to_poly(&aabb2);

        PU_ASSERT(!sat_test_poly_poly(&p1, &p2, NULL));
    }

    return true;
}

PU_TEST(test_aabb_circle_collide)
{
    pm_b2 aabb = pm_b2_make(5, 5, 3, 3);
    sat_poly_t p = sat_aabb_to_poly(&aabb);

    // Right side
    {
        sat_circle_t c = sat_make_circle(pm_v2_make(8, 6.5), 1.0f);

        sat_manifold_t manifold;

        PU_ASSERT(sat_test_poly_circle(&p, &c, &manifold));

        PU_ASSERT(pm_equal(manifold.overlap, 1));
        PU_ASSERT(pm_v2_equal(manifold.normal, pm_v2_make(1, 0)));
        PU_ASSERT(pm_v2_equal(manifold.vector, pm_v2_make(1, 0)));
    }

    // Top
    {

    }

    // On Vertex (TODO: revisit this)
    {
        sat_circle_t c = sat_make_circle(pm_v2_make(5, 5), 1.0f);

        sat_manifold_t manifold;

        PU_ASSERT(sat_test_poly_circle(&p, &c, &manifold));

        PU_ASSERT(pm_equal(manifold.overlap, 1.0f));

        PU_ASSERT(pm_v2_equal(manifold.normal, pm_v2_make(-1,  0)) ||
                  pm_v2_equal(manifold.normal, pm_v2_make( 0, -1)));

        PU_ASSERT(pm_v2_equal(manifold.vector, pm_v2_make(-1,  0)) ||
                  pm_v2_equal(manifold.normal, pm_v2_make( 0, -1)));
    }

    return true;
}

PU_TEST(test_aabb_circle_not_collide)
{
    pm_b2 aabb = pm_b2_make(5, 5, 3, 3);

    sat_poly_t p = sat_aabb_to_poly(&aabb);
    sat_circle_t c = sat_make_circle(pm_v2_make(8, 10), 1.0f);

    PU_ASSERT(!sat_test_poly_circle(&p, &c, NULL));

    return true;
}

PU_TEST(test_circle_cicle_collide)
{
    sat_circle_t c1 = sat_make_circle(pm_v2_make(5, 5), 2.0);
    sat_circle_t c2 = sat_make_circle(pm_v2_make(3, 5), 1.0);

    sat_manifold_t manifold;

    PU_ASSERT(sat_test_circle_circle(&c1, &c2, &manifold));

    PU_ASSERT(pm_equal(manifold.overlap, 1));
    PU_ASSERT(pm_v2_equal(manifold.normal, pm_v2_make(-1, 0)));
    PU_ASSERT(pm_v2_equal(manifold.vector, pm_v2_make(-1, 0)));

    return true;
}

PU_TEST(test_circle_cicle_not_collide)
{
    sat_circle_t c1 = sat_make_circle(pm_v2_make(5, 5), 2.0);
    sat_circle_t c2 = sat_make_circle(pm_v2_make(2, 5), 1.0);

    PU_ASSERT(!sat_test_circle_circle(&c1, &c2, NULL));

    return true;
}

static PU_SUITE(suite_sat)
{
    PU_RUN_TEST(test_aabb_aabb_collide);
    PU_RUN_TEST(test_aabb_aabb_not_collide);
    PU_RUN_TEST(test_aabb_circle_collide);
    PU_RUN_TEST(test_aabb_circle_not_collide);
    PU_RUN_TEST(test_circle_cicle_collide);
    PU_RUN_TEST(test_circle_cicle_not_collide);
}

int main()
{
    pu_display_colors(true);
    PU_RUN_SUITE(suite_sat);
    pu_print_stats();
    return pu_test_failed();
}
