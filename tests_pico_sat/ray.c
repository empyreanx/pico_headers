#include "../pico_unit.h"
#include "../pico_hit.h"

TEST_CASE(test_segment)
{
    pm_v2 r1 = { 0.0f,  0.0f };
    pm_v2 r2 = { 10.0f, 0.0f };

    pm_v2 s1 = { 5.0f,  5.0f };
    pm_v2 s2 = { 5.0f, -1.0f };

    REQUIRE(ph_ray_segment(r1, r2, s1, s2));

    return true;
}

TEST_SUITE(suite_ray)
{
    RUN_TEST_CASE(test_segment);
}