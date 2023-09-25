#include "../pico_math.h"
#include "../pico_unit.h"

TEST_CASE(test_min)
{
    REQUIRE(2 == p_min(2, 4));
    return true;
}

TEST_CASE(test_max)
{
    REQUIRE(4 == p_max(2, 4));
    return true;
}

TEST_CASE(test_clamp)
{
    REQUIRE(-5 == p_clamp(-10, -5, 5));
    REQUIRE( 5 == p_clamp(10, -5, 5));
    REQUIRE( 0 == p_clamp(0, -5, 5));

    return true;
}

TEST_CASE(test_next_pow2)
{
    REQUIRE(16 == p_next_pow2(9));
    REQUIRE(16 == p_next_pow2(8));
    REQUIRE(16 == p_next_pow2(15));

    return true;
}

TEST_CASE(test_lerp_angle)
{
    pfloat angle = p_lerp_angle(0.0f, P_PI / 4.0, 0.5f);
    REQUIRE(p_equal(angle, P_PI / 8.0f));

    angle = p_lerp_angle(P_PI / 4.0, P_PI * 3.0 / 4.0, 0.5f);
    REQUIRE(p_equal(angle, P_PI / 2.0f));

    angle = p_lerp_angle(P_PI / 4.0, P_PI * 3.0 / 4.0, 0.0f);
    REQUIRE(p_equal(angle, P_PI / 4.0f));

    angle = p_lerp_angle(P_PI / 4.0, P_PI * 3.0 / 4.0, 1.0f);
    REQUIRE(p_equal(angle, P_PI * 3.0 / 4.0));

    //NOTE: This pathological
    //angle = p_lerp_angle(P_PI / 4.0f, P_PI * 7.0f / 4.0f, 0.5);
    //REQUIRE(p_equal(angle, 0.0f));

    //angle = p_lerp_angle(P_PI / 4.0f, P_PI * 7.0f / 4.0f, 0.5);
    //REQUIRE(p_equal(angle, P_PI));

    angle = p_lerp_angle(P_PI * 7.0f / 4.0f, P_PI / 4.0f, 0.75);
    REQUIRE(p_equal(angle, P_PI / 8.0f));

    return true;
}

TEST_SUITE(suite_scalar)
{
    RUN_TEST_CASE(test_min);
    RUN_TEST_CASE(test_max);
    RUN_TEST_CASE(test_clamp);
    RUN_TEST_CASE(test_next_pow2);
    RUN_TEST_CASE(test_lerp_angle);
}
