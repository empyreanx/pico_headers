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
        REQUIRE(pv2_equal(manifold.normal, pv2_make(-1, 0)));
        REQUIRE(pv2_equal(manifold.vector, pv2_make(-1, 0)));
    }

    // Collide left side
    {
        pb2 aabb2 = pb2_make(4, 5, 2, 2);
        ph_poly_t p2 = ph_aabb_to_poly(&aabb2);

        ph_manifold_t manifold;

        REQUIRE(ph_sat_poly_poly(&p1, &p2, &manifold));

        REQUIRE(pf_equal(manifold.overlap, 1));
        REQUIRE(pv2_equal(manifold.normal, pv2_make(1, 0)));
        REQUIRE(pv2_equal(manifold.vector, pv2_make(1, 0)));
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

    ph_poly_t p1 = ph_make_poly(vertices1, 4, false);
    ph_poly_t p2 = ph_make_poly(vertices2, 3, false);

    ph_manifold_t manifold;

    REQUIRE(ph_sat_poly_poly(&p1, &p2, &manifold));

    REQUIRE(pf_equal(manifold.overlap, 10));
    REQUIRE(pv2_equal(manifold.normal, pv2_make(-1, 0)));

    return true;
}


TEST_CASE(test_poly_poly_mtv)
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

    ph_poly_t p1 = ph_make_poly(vertices1, 4, false);
    ph_poly_t p2 = ph_make_poly(vertices2, 3, false);
    ph_manifold_t manifold;

    REQUIRE(ph_sat_poly_poly(&p1, &p2, &manifold));
    pt2 tf = pt2_translation(manifold.vector);
    ph_poly_t tf_poly = ph_transform_poly(&tf, &p1);

    REQUIRE(!ph_sat_poly_poly(&tf_poly, &p2, NULL));

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

TEST_CASE(test_poly_to_aabb)
{
    const pv2 vertices[] =
    {
        {  2, 5 },
        { -4, 3 },
        {  5, 1 }
    };

    ph_poly_t p = ph_make_poly(vertices, 3, false);

    pb2 exp = pb2_make_minmax(pv2_make(-4, 1), pv2_make(5, 5));
    pb2 res = ph_poly_to_aabb(&p);

    REQUIRE(pb2_equal(&exp, &res));

    return true;
}


TEST_SUITE(suite_poly_poly)
{
    RUN_TEST_CASE(test_aabb_aabb_collide);
    RUN_TEST_CASE(test_aabb_aabb_not_collide);
    RUN_TEST_CASE(test_poly_poly);
    RUN_TEST_CASE(test_poly_poly_mtv);
    RUN_TEST_CASE(test_poly_to_aabb);
}
