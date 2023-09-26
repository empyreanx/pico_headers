#include "../pico_unit.h"
#include "../pico_hit.h"

TEST_CASE(test_segment_hit)
{
    ph_ray_t r = ph_make_ray(pv2_make(0.f, 0.f), pv2_make(1.f, 0.f), 10.f);

    pv2 s1 = { 5.f,  5.f };
    pv2 s2 = { 5.f, -1.f };

    REQUIRE(ph_ray_line(&r, s1, s2, NULL));

    r = ph_make_ray(pv2_make(0.f, 0.f), pv2_normalize(pv2_make(1.f, 1.f)), 10.f);

    s1 = (pv2){ 0.f,  5.f };
    s2 = (pv2){ 5.f, -10.f };

    REQUIRE(ph_ray_line(&r, s1, s2, NULL));

    return true;
}

TEST_CASE(test_segment_no_hit)
{
    ph_ray_t r = ph_make_ray(pv2_make(0.f, 0.f), pv2_make(1.f, 0.f), 10.f);

    pv2 s1 = { 5.0f,  5.0f };
    pv2 s2 = { 5.0f,  2.0f };

    REQUIRE(!ph_ray_line(&r, s1, s2, NULL));

    s1 = (pv2){ 5.0f,  5.0f };
    s2 = (pv2){ 5.0f, 10.0f };

    REQUIRE(!ph_ray_line(&r, s1, s2, NULL));

    return true;
}

TEST_CASE(test_segment_raycast)
{
    { // Case 1

        pfloat dist = pf_sqrt(pf_pow(10.f, 2.f) + pf_pow(10.f, 2.f));
        ph_ray_t r = ph_make_ray(pv2_make(0.f, 0.f), pv2_normalize(pv2_make(1.f, 1.f)), dist);

        pv2 s1 = { 0.0f, 10.0f };
        pv2 s2 = { 10.0f, 0.0f };

        ph_raycast_t raycast;

        REQUIRE(ph_ray_line(&r, s1, s2, &raycast));

        pv2 normal = pv2_normalize(pv2_make(1.f, 1.f));

        REQUIRE(pv2_equal(raycast.normal, normal));

        REQUIRE(pf_equal(raycast.dist, dist * .5f));
    }

    { // Case 2

        ph_ray_t r = ph_make_ray(pv2_make(7.5f, 7.5f), pv2_normalize(pv2_make(0.f, -1.f)), 7.5f);

        pv2 s1 = { 0.0f, 0.5f };
        pv2 s2 = { 10.0f, 0.5f };

        ph_raycast_t raycast;

        REQUIRE(ph_ray_line(&r, s1, s2, &raycast));

        pv2 normal = pv2_normalize(pv2_make(0.f, 1.f));

        REQUIRE(pv2_equal(raycast.normal, normal));

        REQUIRE(pf_equal(raycast.dist, 7.f));
    }

    return true;
}

TEST_CASE(test_poly_hit)
{
    ph_poly_t poly = ph_aabb_to_poly(&pb2_make(2.5f, 2.5f, 2.5f, 2.5f));

    {   // Left
        ph_ray_t ray = ph_make_ray(pv2_make(0.f, 3.f), pv2_make(1.f, 0.f), 10.f);
        REQUIRE(ph_ray_poly(&ray, &poly, NULL));
    }

    {   // Right
        ph_ray_t ray = ph_make_ray(pv2_make(7.f, 3.f), pv2_make(-1.f, 0.f), 10.f);
        REQUIRE(ph_ray_poly(&ray, &poly, NULL));
    }

    {   // Top
        ph_ray_t ray = ph_make_ray(pv2_make(3.f, 0.f), pv2_make(0.f, 1.f), 10.f);
        REQUIRE(ph_ray_poly(&ray, &poly, NULL));
    }

    {   // Bottom
        ph_ray_t ray = ph_make_ray(pv2_make(3.f, 7.f), pv2_make(0.f, -1.f), 10.f);
        REQUIRE(ph_ray_poly(&ray, &poly, NULL));
    }

    return true;
}

TEST_CASE(test_poly_no_hit)
{
    ph_poly_t poly = ph_aabb_to_poly(&pb2_make(2.5f, 2.5f, 2.5f, 2.5f));

    {   // Left
        ph_ray_t ray = ph_make_ray(pv2_make(0.f, 3.f), pv2_make(1.f, 2.f), 10.f);
        REQUIRE(!ph_ray_poly(&ray, &poly, NULL));
    }

    {   // Right
        ph_ray_t ray = ph_make_ray(pv2_make(7.f, 3.f), pv2_make(1.f, 0.f), 10.f);
        REQUIRE(!ph_ray_poly(&ray, &poly, NULL));
    }

    {   // Top
        ph_ray_t ray = ph_make_ray(pv2_make(3.f, 0.f), pv2_make(0.f, -1.f), 10.f);
        REQUIRE(!ph_ray_poly(&ray, &poly, NULL));
    }

    {   // Bottom
        ph_ray_t ray = ph_make_ray(pv2_make(3.f, 7.f), pv2_make(0.f, 1.f), 10.f);
        REQUIRE(!ph_ray_poly(&ray, &poly, NULL));
    }

    return true;
}

TEST_CASE(test_poly_raycast)
{
    ph_poly_t poly = ph_aabb_to_poly(&pb2_make(2.5f, 2.5f, 2.5f, 2.5f));

    {   // Left
        ph_ray_t ray = ph_make_ray(pv2_make(0.f, 3.f), pv2_make(1.f, 0.f), 10.f);
        ph_raycast_t raycast;
        REQUIRE(ph_ray_poly(&ray, &poly, &raycast));
        REQUIRE(pv2_equal(raycast.normal, pv2_make(-1.f, 0.f)));
        REQUIRE(pf_equal(raycast.dist, 2.5f));
    }

    {   // Right
        ph_ray_t ray = ph_make_ray(pv2_make(7.f, 3.f), pv2_make(-1.f, 0.f), 10.f);
        ph_raycast_t raycast;
        REQUIRE(ph_ray_poly(&ray, &poly, &raycast));
        REQUIRE(pv2_equal(raycast.normal, pv2_make(1.f, 0.f)));
        REQUIRE(pf_equal(raycast.dist, 2.f));
    }

    {   // Top
        ph_ray_t ray = ph_make_ray(pv2_make(3.f, 0.f), pv2_make(0.f, 1.f), 10.f);
        ph_raycast_t raycast;
        REQUIRE(ph_ray_poly(&ray, &poly, &raycast));
        REQUIRE(pv2_equal(raycast.normal, pv2_make(0.f, -1.f)));
        REQUIRE(pf_equal(raycast.dist, 2.5f));
    }

    {   // Bottom
        ph_ray_t ray = ph_make_ray(pv2_make(3.f, 7.f), pv2_make(0.f, -1.f), 10.f);
        ph_raycast_t raycast;
        REQUIRE(ph_ray_poly(&ray, &poly, &raycast));
        REQUIRE(pv2_equal(raycast.normal, pv2_make(0.f, 1.f)));
        REQUIRE(pf_equal(raycast.dist, 2.f));
    }

    return true;
}

TEST_CASE(test_circle_hit)
{
    ph_circle_t circle = ph_make_circle(pv2_make(5.f, 5.f), 2.f);

    { // Case 1
        ph_ray_t ray = ph_make_ray(pv2_make(0.f, 5.f), pv2_make(1.f, 0.f), 5.f);
        REQUIRE(ph_ray_circle(&ray, &circle, NULL));
    }

    { // Case 2
        ph_ray_t ray = ph_make_ray(pv2_make(0.f, 5.f), pv2_make(5.f, 1.f), 5.f);
        REQUIRE(ph_ray_circle(&ray, &circle, NULL));
    }

    return true;
}

TEST_CASE(test_circle_no_hit)
{
    ph_circle_t circle = ph_make_circle(pv2_make(5.f, 5.f), 2.f);

    { // Case 1
        ph_ray_t ray = ph_make_ray(pv2_make(0.f, 5.f), pv2_make(-1.f, 0.f), 5.f);
        REQUIRE(!ph_ray_circle(&ray, &circle, NULL));
    }

    { // Case 2
        ph_ray_t ray = ph_make_ray(pv2_make(0.f, 5.f), pv2_make(1.f, 3.f), 5.f);
        REQUIRE(!ph_ray_circle(&ray, &circle, NULL));
    }

    return true;
}

TEST_CASE(test_circle_raycast)
{
    ph_circle_t circle = ph_make_circle(pv2_make(5.f, 5.f), 2.f);

    { // Case 1
        ph_raycast_t raycast;
        ph_ray_t ray = ph_make_ray(pv2_make(0.f, 5.f), pv2_make(1.f, 0.f), 5.f);
        REQUIRE(ph_ray_circle(&ray, &circle, &raycast));
        REQUIRE(pv2_equal(raycast.normal, pv2_make(-1.f, 0.f)));
        REQUIRE(pf_equal(raycast.dist, 3.f));
    }

    return true;
}

TEST_CASE(test_ray_at)
{
    ph_ray_t ray = ph_make_ray(pv2_make(0.f, 0.f), pv2_make(1.f, 1.f), 0.f);
    pv2 point = ph_ray_at(&ray, pf_sqrt(200.f));

    REQUIRE(pv2_equal(point, pv2_make(10.f, 10.f)));

    return true;
}

TEST_SUITE(suite_ray)
{
    RUN_TEST_CASE(test_segment_hit);
    RUN_TEST_CASE(test_segment_no_hit);
    RUN_TEST_CASE(test_segment_raycast);
    RUN_TEST_CASE(test_poly_hit);
    RUN_TEST_CASE(test_poly_no_hit);
    RUN_TEST_CASE(test_poly_raycast);
    RUN_TEST_CASE(test_circle_hit);
    RUN_TEST_CASE(test_circle_no_hit);
    RUN_TEST_CASE(test_circle_raycast);
    RUN_TEST_CASE(test_ray_at);
}
