#include <stdio.h>

#define PICO_SAT_IMPLEMENTATION
#include "../pico_sat.h"

#define PICO_MATH_IMPLEMENTATION
#include "../pico_math.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

PU_TEST(test_collide)
{
    return true;
}

static PU_SUITE(suite_sat)
{
    PU_RUN_TEST(test_collide);
}

int main()
{
    pu_display_colors(true);
    PU_RUN_SUITE(suite_sat);
    pu_print_stats();
    return pu_test_failed();
}
