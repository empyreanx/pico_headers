#include "../pico_hit.h"
#include "../pico_unit.h"

TEST_CASE(test_circle_cicle_collide)
{
    ph_circle_t c1 = ph_make_circle(pv2_make(5, 5), 2.0);
    ph_circle_t c2 = ph_make_circle(pv2_make(3, 5), 1.0);

    ph_sat_t result;

    REQUIRE(ph_sat_circle_circle(&c1, &c2, &result));

    REQUIRE(pf_equal(result.overlap, 1));
    REQUIRE(pv2_equal(result.normal, pv2_make(1, 0)));
    REQUIRE(pv2_equal(result.mtv, pv2_make(1, 0)));

    return true;
}

TEST_CASE(test_circle_cicle_not_collide)
{
    ph_circle_t c1 = ph_make_circle(pv2_make(5, 5), 2.0);
    ph_circle_t c2 = ph_make_circle(pv2_make(2, 5), 1.0);

    REQUIRE(!ph_sat_circle_circle(&c1, &c2, NULL));

    return true;
}

TEST_SUITE(suite_circle_circle)
{
    RUN_TEST_CASE(test_circle_cicle_collide);
    RUN_TEST_CASE(test_circle_cicle_not_collide);
}
