#include "../pico_unit.h"
#include "../pico_hit.h"

TEST_CASE(test_segment_hit)
{
    pm_v2 r1 = { 0.0f,  0.0f };
    pm_v2 r2 = { 10.0f, 0.0f };

    pm_v2 s1 = { 5.0f,  5.0f };
    pm_v2 s2 = { 5.0f, -1.0f };

    REQUIRE(ph_ray_segment(r1, r2, s1, s2, NULL));

    r1 = (pm_v2){ 0.0f,  0.0f  };
    r2 = (pm_v2){ 10.0f, 10.0f };

    s1 = (pm_v2){ 0.0f,  5.0f };
    s2 = (pm_v2){ 5.0f, -10.0f };

    return true;
}

TEST_CASE(test_segment_no_hit)
{

    pm_v2 r1 = { 0.0f,  0.0f };
    pm_v2 r2 = { 10.0f, 0.0f };

    pm_v2 s1 = { 5.0f,  5.0f };
    pm_v2 s2 = { 5.0f,  2.0f };

    REQUIRE(!ph_ray_segment(r1, r2, s1, s2, NULL));

    s1 = (pm_v2){ 5.0f,  5.0f };
    s2 = (pm_v2){ 5.0f, 10.0f };

    REQUIRE(!ph_ray_segment(r1, r2, s1, s2, NULL));

    return true;
}

TEST_CASE(test_segment_raycast)
{
    pm_v2 r1 = { 0.0f,  0.0f  };
    pm_v2 r2 = { 10.0f, 10.0f };

    pm_v2 s1 = { 0.0f, 10.0f };
    pm_v2 s2 = { 10.0f, 0.0f };

    ph_raycast_t raycast;

    REQUIRE(ph_ray_segment(r1, r2, s1, s2, &raycast));

    pm_v2 normal = pm_v2_normalize(pm_v2_make(1.f, 1.f));

    REQUIRE(pm_v2_equal(raycast.normal, normal));

    REQUIRE(pm_equal(raycast.alpha, .5f));

    return true;
}

TEST_SUITE(suite_ray)
{
    RUN_TEST_CASE(test_segment_hit);
    RUN_TEST_CASE(test_segment_no_hit);
    RUN_TEST_CASE(test_segment_raycast);
}