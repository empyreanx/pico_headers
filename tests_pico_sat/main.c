#include <stdio.h>

#define PICO_SAT_IMPLEMENTATION
#include "../pico_sat.h"

#define PICO_MATH_IMPLEMENTATION
#include "../pico_math.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

PU_TEST(test_collide)
{
    pm_b2 aabb1 = pm_b2_make(5, 5, 2, 2);
    pm_b2 aabb2 = pm_b2_make(6, 6, 2, 2);

    sat_poly_t p1 = sat_aabb_to_poly(&aabb1);
    sat_poly_t p2 = sat_aabb_to_poly(&aabb2);

    PU_ASSERT(sat_test_poly_poly(&p1, &p2, NULL));

    return true;
}

PU_TEST(test_not_collide)
{
    pm_b2 aabb1 = pm_b2_make(5, 5, 2, 2);
    pm_b2 aabb2 = pm_b2_make(9, 9, 2, 2);

    sat_poly_t p1 = sat_aabb_to_poly(&aabb1);
    sat_poly_t p2 = sat_aabb_to_poly(&aabb2);

    PU_ASSERT(!sat_test_poly_poly(&p1, &p2, NULL));

    return true;
}

static PU_SUITE(suite_sat)
{
    PU_RUN_TEST(test_collide);
    PU_RUN_TEST(test_not_collide);
}

int main()
{
    pu_display_colors(true);
    PU_RUN_SUITE(suite_sat);
    pu_print_stats();
    return pu_test_failed();
}
