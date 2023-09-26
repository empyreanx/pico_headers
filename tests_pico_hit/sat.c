#include "../pico_hit.h"
#include "../pico_unit.h"

TEST_CASE(test_aabb_aabb_collide)
{
    pb2 aabb1 = pb2_make(5, 5, 2, 2);
    ph_poly_t p1 = ph_aabb_to_poly(&aabb1);

    // Collide right side
    {
        pb2 aabb2 = pb2_make(6, 6, 2, 2);
        ph_poly_t p2 = ph_aabb_to_poly(&aabb2);

        ph_manifold_t manifold;

        REQUIRE(ph_sat_poly_poly(&p1, &p2, &manifold));

        REQUIRE(pf_equal(manifold.overlap, 1));
        REQUIRE(pv2_equal(manifold.normal, pv2_make(1, 0)));
        REQUIRE(pv2_equal(manifold.vector, pv2_make(1, 0)));
    }

    // Collide left side
    {
        pb2 aabb2 = pb2_make(4, 5, 2, 2);
        ph_poly_t p2 = ph_aabb_to_poly(&aabb2);

        ph_manifold_t manifold;

        REQUIRE(ph_sat_poly_poly(&p1, &p2, &manifold));

        REQUIRE(pf_equal(manifold.overlap, 1));
        REQUIRE(pv2_equal(manifold.normal, pv2_make(-1, 0)));
        REQUIRE(pv2_equal(manifold.vector, pv2_make(-1, 0)));
    }

    return true;
}

TEST_CASE(test_poly_poly)
{
    pv2 vertices1[] =
    {
        { 0,  0  },
        { 0,  40 },
        { 40, 40 },
        { 40, 0  }
    };

    pv2 vertices2[] =
    {
        { 30, 0  },
        { 30, 30 },
        { 60, 0  }
    };

    ph_poly_t p1 = ph_make_poly(vertices1, 4);
    ph_poly_t p2 = ph_make_poly(vertices2, 3);

    ph_manifold_t manifold;

    REQUIRE(ph_sat_poly_poly(&p1, &p2, &manifold));

    REQUIRE(pf_equal(manifold.overlap, 10));
    REQUIRE(pv2_equal(manifold.normal, pv2_make(1, 0)));

    return true;
}

TEST_CASE(test_aabb_aabb_not_collide)
{
    pb2 aabb1 = pb2_make(5, 5, 2, 2);
    ph_poly_t p1 = ph_aabb_to_poly(&aabb1);

    // Diagonal
    {
        pb2 aabb2 = pb2_make(9, 9, 2, 2);
        ph_poly_t p2 = ph_aabb_to_poly(&aabb2);

        REQUIRE(!ph_sat_poly_poly(&p1, &p2, NULL));
    }

    // Left
    {
        pb2 aabb2 = pb2_make(3, 5, 2, 2);
        ph_poly_t p2 = ph_aabb_to_poly(&aabb2);

        REQUIRE(!ph_sat_poly_poly(&p1, &p2, NULL));
    }

    return true;
}

TEST_CASE(test_aabb_circle_collide)
{
    pb2 aabb = pb2_make(5, 5, 3, 3);
    ph_poly_t p = ph_aabb_to_poly(&aabb);

    // Right side
    {
        ph_circle_t c = ph_make_circle(pv2_make(8, 6.5), 1.0f);

        ph_manifold_t manifold;

        REQUIRE(ph_sat_poly_circle(&p, &c, &manifold));

        REQUIRE(pf_equal(manifold.overlap, 1));
        REQUIRE(pv2_equal(manifold.normal, pv2_make(1, 0)));
        REQUIRE(pv2_equal(manifold.vector, pv2_make(1, 0)));
    }

    // Top
    {

    }

    // On Vertex
    {
        ph_circle_t c = ph_make_circle(pv2_make(5, 5), 1.0f);

        ph_manifold_t manifold;

        REQUIRE(ph_sat_poly_circle(&p, &c, &manifold));

        REQUIRE(pf_equal(manifold.overlap, 1.0f));

        REQUIRE(pv2_equal(manifold.normal, pv2_make(-1,  0)) ||
                pv2_equal(manifold.normal, pv2_make( 0, -1)));

        REQUIRE(pv2_equal(manifold.vector, pv2_make(-1,  0)) ||
                pv2_equal(manifold.normal, pv2_make( 0, -1)));
    }

    return true;
}

TEST_CASE(test_aabb_circle_not_collide)
{
    pb2 aabb = pb2_make(5, 5, 3, 3);

    ph_poly_t p = ph_aabb_to_poly(&aabb);
    ph_circle_t c = ph_make_circle(pv2_make(8, 10), 1.0f);

    REQUIRE(!ph_sat_poly_circle(&p, &c, NULL));

    return true;
}

TEST_CASE(test_circle_cicle_collide)
{
    ph_circle_t c1 = ph_make_circle(pv2_make(5, 5), 2.0);
    ph_circle_t c2 = ph_make_circle(pv2_make(3, 5), 1.0);

    ph_manifold_t manifold;

    REQUIRE(ph_sat_circle_circle(&c1, &c2, &manifold));

    REQUIRE(pf_equal(manifold.overlap, 1));
    REQUIRE(pv2_equal(manifold.normal, pv2_make(-1, 0)));
    REQUIRE(pv2_equal(manifold.vector, pv2_make(-1, 0)));

    return true;
}

TEST_CASE(test_circle_cicle_not_collide)
{
    ph_circle_t c1 = ph_make_circle(pv2_make(5, 5), 2.0);
    ph_circle_t c2 = ph_make_circle(pv2_make(2, 5), 1.0);

    REQUIRE(!ph_sat_circle_circle(&c1, &c2, NULL));

    return true;
}

TEST_CASE(test_circle_to_aabb)
{
    ph_circle_t c = ph_make_circle(pv2_make(0, 0), 1);

    pb2 exp = pb2_make(-0.5f, -0.5f, 1, 1);
    pb2 res = ph_circle_to_aabb(&c);

    REQUIRE(pb2_equal(&exp, &res));

    return true;
}

TEST_CASE(test_poly_to_aabb)
{
    const pv2 vertices[] =
    {
        {  2, 5 },
        { -4, 3 },
        {  5, 1 }
    };

    ph_poly_t p = ph_make_poly(vertices, 3);

    pb2 exp = pb2_make_minmax(pv2_make(-4, 1), pv2_make(5, 5));
    pb2 res = ph_poly_to_aabb(&p);

    REQUIRE(pb2_equal(&exp, &res));

    return true;
}

TEST_CASE(test_transform_poly)
{
    pb2 b = pb2_make(-0.5f, -0.5f, 1, 1);
    ph_poly_t p = ph_aabb_to_poly(&b);

    pt2 t = pt2_identity();
    pt2_rotate(&t, PM_PI / 4.0f);

    ph_poly_t res = ph_transform_poly(&t, &p);

    const pfloat half_diag = 0.5f * pf_sqrt(2.0f);

    REQUIRE(pv2_equal(res.vertices[0], pv2_make(0.0f, -half_diag)));
    REQUIRE(pv2_equal(res.vertices[1], pv2_make(-half_diag, 0.0f)));
    REQUIRE(pv2_equal(res.vertices[2], pv2_make(0.0f, half_diag)));
    REQUIRE(pv2_equal(res.vertices[3], pv2_make(half_diag, 0.0f)));

    return true;
}

TEST_CASE(test_transform_circle)
{
    ph_circle_t c = ph_make_circle(pv2_make(1, 0), 1);

    pt2 t = pt2_identity();
    pt2_rotate(&t, PM_PI / 2.0f);
    pt2_translate(&t, pv2_make(0, 1));

    ph_circle_t res = ph_transform_circle(&t, &c);

    REQUIRE(pv2_equal(res.pos, pv2_make(0, 2)));

    return true;
}

TEST_SUITE(suite_sat)
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
