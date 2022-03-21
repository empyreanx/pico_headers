#define PM_IMPLEMENTATION
#include "../pico_math.h"

#define PU_IMPLEMENTATION
#include "../pico_unit.h"

void suite_scalar();
void suite_v2();
void suite_t2();
void suite_b2();
void suite_rng();

int main()
{
    pu_display_colors(true);

    PU_RUN_SUITE(suite_scalar);
    PU_RUN_SUITE(suite_v2);
    PU_RUN_SUITE(suite_t2);
    PU_RUN_SUITE(suite_b2);
    PU_RUN_SUITE(suite_rng);

    pu_print_stats();

    return pu_test_failed();
}
