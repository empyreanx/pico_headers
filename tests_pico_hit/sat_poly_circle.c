#include "../pico_hit.h"
#include "../pico_unit.h"

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
        REQUIRE(pv2_equal(manifold.normal, pv2_make(-1, 0)));
        REQUIRE(pv2_equal(manifold.vector, pv2_make(-1, 0)));
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

        REQUIRE(pv2_equal(manifold.normal, pv2_make( 1, 0)) ||
                pv2_equal(manifold.normal, pv2_make( 0, 1)));

        REQUIRE(pv2_equal(manifold.vector, pv2_make( 1, 0)) ||
                pv2_equal(manifold.vector, pv2_make( 0, 1)));
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

TEST_CASE(test_irregular_poly_circle)
{
#if 0
    // world
    pv2 vertices[] =
    {
        { 118.085091, 39.956306 },
        { 107.585091, 33.356304 },
        { 84.635086, 34.106304  },
        { 90.185089, 89.156311  },
        { 107.435089, 89.306305 },
        { 118.685089, 51.356304 },
    };
#endif

//#if 0
    // local
    pv2 vertices[] =
    {
        { 177.000000, -132.000000 },
        { 107.000000, -176.000000 },
        { -46.000000, -171.000000 },
        { -9.000000,   196.000000 },
        { 106.000000,  197.000000 },
        { 181.000000,  -56.000000 },
    };
//#endif

    ph_poly_t poly = ph_make_poly(vertices, sizeof(vertices)/sizeof(vertices[0]), false);

    pt2 scaling_tf = pt2_scaling(pv2_make(0.15, 0.15));
    pt2 translate_tf = pt2_translation(pv2_make(93.639587, 60.062496));
    pt2 tf = pt2_mult(&translate_tf, &scaling_tf);
    poly = ph_transform_poly(&tf, &poly);

    ph_circle_t circle = ph_make_circle(pv2_make(100, 100), 20);

    ph_manifold_t manifold = { 0 };

    REQUIRE(ph_sat_poly_circle(&poly, &circle, &manifold));

    return true;
}

TEST_SUITE(suite_poly_circle)
{
    RUN_TEST_CASE(test_aabb_circle_collide);
    RUN_TEST_CASE(test_aabb_circle_not_collide);
    RUN_TEST_CASE(test_irregular_poly_circle);
}
