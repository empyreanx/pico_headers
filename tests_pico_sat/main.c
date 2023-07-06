#include <stdio.h>

#include "../pico_hit.h"
#include "../pico_math.h"
#include "../pico_unit.h"

TEST_CASE(test_aabb_aabb_collide)
{
    pm_b2 aabb1 = pm_b2_make(5, 5, 2, 2);
    ph_poly_t p1 = ph_aabb_to_poly(&aabb1);

    // Collide right side
    {
        pm_b2 aabb2 = pm_b2_make(6, 6, 2, 2);
        ph_poly_t p2 = ph_aabb_to_poly(&aabb2);

        ph_manifold_t manifold;

        REQUIRE(ph_sat_poly_poly(&p1, &p2, &manifold));

        REQUIRE(pm_equal(manifold.overlap, 1));
        REQUIRE(pm_v2_equal(manifold.normal, pm_v2_make(1, 0)));
        REQUIRE(pm_v2_equal(manifold.vector, pm_v2_make(1, 0)));
    }

    // Collide left side
    {
        pm_b2 aabb2 = pm_b2_make(4, 5, 2, 2);
        ph_poly_t p2 = ph_aabb_to_poly(&aabb2);

        ph_manifold_t manifold;

        REQUIRE(ph_sat_poly_poly(&p1, &p2, &manifold));

        REQUIRE(pm_equal(manifold.overlap, 1));
        REQUIRE(pm_v2_equal(manifold.normal, pm_v2_make(-1, 0)));
        REQUIRE(pm_v2_equal(manifold.vector, pm_v2_make(-1, 0)));
    }

    return true;
}

TEST_CASE(test_poly_poly)
{
    pm_v2 vertices1[] =
    {
        { 0,  0  },
        { 0,  40 },
        { 40, 40 },
        { 40, 0  }
    };

    pm_v2 vertices2[] =
    {
        { 30, 0  },
        { 30, 30 },
        { 60, 0  }
    };

    ph_poly_t p1 = ph_make_poly(vertices1, 4);
    ph_poly_t p2 = ph_make_poly(vertices2, 3);

    ph_manifold_t manifold;

    REQUIRE(ph_sat_poly_poly(&p1, &p2, &manifold));

    REQUIRE(pm_equal(manifold.overlap, 10));
    REQUIRE(pm_v2_equal(manifold.normal, pm_v2_make(1, 0)));

    return true;
}

TEST_CASE(test_aabb_aabb_not_collide)
{
    pm_b2 aabb1 = pm_b2_make(5, 5, 2, 2);
    ph_poly_t p1 = ph_aabb_to_poly(&aabb1);

    // Diagonal
    {
        pm_b2 aabb2 = pm_b2_make(9, 9, 2, 2);
        ph_poly_t p2 = ph_aabb_to_poly(&aabb2);

        REQUIRE(!ph_sat_poly_poly(&p1, &p2, NULL));
    }

    // Left
    {
        pm_b2 aabb2 = pm_b2_make(3, 5, 2, 2);
        ph_poly_t p2 = ph_aabb_to_poly(&aabb2);

        REQUIRE(!ph_sat_poly_poly(&p1, &p2, NULL));
    }

    return true;
}

TEST_CASE(test_aabb_circle_collide)
{
    pm_b2 aabb = pm_b2_make(5, 5, 3, 3);
    ph_poly_t p = ph_aabb_to_poly(&aabb);

    // Right side
    {
        ph_circle_t c = ph_make_circle(pm_v2_make(8, 6.5), 1.0f);

        ph_manifold_t manifold;

        REQUIRE(ph_sat_poly_circle(&p, &c, &manifold));

        REQUIRE(pm_equal(manifold.overlap, 1));
        REQUIRE(pm_v2_equal(manifold.normal, pm_v2_make(1, 0)));
        REQUIRE(pm_v2_equal(manifold.vector, pm_v2_make(1, 0)));
    }

    // Top
    {

    }

    // On Vertex
    {
        ph_circle_t c = ph_make_circle(pm_v2_make(5, 5), 1.0f);

        ph_manifold_t manifold;

        REQUIRE(ph_sat_poly_circle(&p, &c, &manifold));

        REQUIRE(pm_equal(manifold.overlap, 1.0f));

        REQUIRE(pm_v2_equal(manifold.normal, pm_v2_make(-1,  0)) ||
                  pm_v2_equal(manifold.normal, pm_v2_make( 0, -1)));

        REQUIRE(pm_v2_equal(manifold.vector, pm_v2_make(-1,  0)) ||
                  pm_v2_equal(manifold.normal, pm_v2_make( 0, -1)));
    }

    return true;
}

TEST_CASE(test_aabb_circle_not_collide)
{
    pm_b2 aabb = pm_b2_make(5, 5, 3, 3);

    ph_poly_t p = ph_aabb_to_poly(&aabb);
    ph_circle_t c = ph_make_circle(pm_v2_make(8, 10), 1.0f);

    REQUIRE(!ph_sat_poly_circle(&p, &c, NULL));

    return true;
}

TEST_CASE(test_circle_cicle_collide)
{
    ph_circle_t c1 = ph_make_circle(pm_v2_make(5, 5), 2.0);
    ph_circle_t c2 = ph_make_circle(pm_v2_make(3, 5), 1.0);

    ph_manifold_t manifold;

    REQUIRE(ph_sat_circle_circle(&c1, &c2, &manifold));

    REQUIRE(pm_equal(manifold.overlap, 1));
    REQUIRE(pm_v2_equal(manifold.normal, pm_v2_make(-1, 0)));
    REQUIRE(pm_v2_equal(manifold.vector, pm_v2_make(-1, 0)));

    return true;
}

TEST_CASE(test_circle_cicle_not_collide)
{
    ph_circle_t c1 = ph_make_circle(pm_v2_make(5, 5), 2.0);
    ph_circle_t c2 = ph_make_circle(pm_v2_make(2, 5), 1.0);

    REQUIRE(!ph_sat_circle_circle(&c1, &c2, NULL));

    return true;
}

TEST_CASE(test_circle_to_aabb)
{
    ph_circle_t c = ph_make_circle(pm_v2_make(0, 0), 1);

    pm_b2 exp = pm_b2_make(-0.5f, -0.5f, 1, 1);
    pm_b2 res = ph_circle_to_aabb(&c);

    REQUIRE(pm_b2_equal(&exp, &res));

    return true;
}

TEST_CASE(test_poly_to_aabb)
{
    const pm_v2 vertices[] =
    {
        {  2, 5 },
        { -4, 3 },
        {  5, 1 }
    };

    ph_poly_t p = ph_make_poly(vertices, 3);

    pm_b2 exp = pm_b2_make_minmax(pm_v2_make(-4, 1), pm_v2_make(5, 5));
    pm_b2 res = ph_poly_to_aabb(&p);

    REQUIRE(pm_b2_equal(&exp, &res));

    return true;
}

TEST_CASE(test_transform_poly)
{
    pm_b2 b = pm_b2_make(-0.5f, -0.5f, 1, 1);
    ph_poly_t p = ph_aabb_to_poly(&b);

    pm_t2 t = pm_t2_identity();
    pm_t2_rotate(&t, PM_PI / 4.0f);

    ph_poly_t res = ph_transform_poly(&t, &p);

    const pm_float half_diag = 0.5f * pm_sqrt(2.0f);

    REQUIRE(pm_v2_equal(res.vertices[0], pm_v2_make(0.0f, -half_diag)));
    REQUIRE(pm_v2_equal(res.vertices[1], pm_v2_make(-half_diag, 0.0f)));
    REQUIRE(pm_v2_equal(res.vertices[2], pm_v2_make(0.0f, half_diag)));
    REQUIRE(pm_v2_equal(res.vertices[3], pm_v2_make(half_diag, 0.0f)));

    return true;
}

TEST_CASE(test_transform_circle)
{
    ph_circle_t c = ph_make_circle(pm_v2_make(1, 0), 1);

    pm_t2 t = pm_t2_identity();
    pm_t2_rotate(&t, PM_PI / 2.0f);
    pm_t2_translate(&t, pm_v2_make(0, 1));

    ph_circle_t res = ph_transform_circle(&t, &c);

    REQUIRE(pm_v2_equal(res.pos, pm_v2_make(0, 2)));

    return true;
}

static TEST_SUITE(suite_sat)
{
    RUN_TEST_CASE(test_aabb_aabb_collide);
    RUN_TEST_CASE(test_aabb_aabb_not_collide);
    RUN_TEST_CASE(test_poly_poly);
    RUN_TEST_CASE(test_aabb_circle_collide);
    RUN_TEST_CASE(test_aabb_circle_not_collide);
    RUN_TEST_CASE(test_circle_cicle_collide);
    RUN_TEST_CASE(test_circle_cicle_not_collide);
    RUN_TEST_CASE(test_circle_to_aabb);
    RUN_TEST_CASE(test_poly_to_aabb);
    RUN_TEST_CASE(test_transform_poly);
    RUN_TEST_CASE(test_transform_circle);
}

TEST_SUITE(suite_ray);

int main()
{
    pu_display_colors(true);
    RUN_TEST_SUITE(suite_sat);
    RUN_TEST_SUITE(suite_ray);
    pu_print_stats();
    return pu_test_failed();
}

#define PICO_HIT_IMPLEMENTATION
#include "../pico_hit.h"

#define PICO_MATH_IMPLEMENTATION
#include "../pico_math.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"
