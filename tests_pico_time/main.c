#define PICO_TIME_IMPLEMENTATION
#include "../pico_time.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

PU_TEST(test_sleep)
{
    ptime_t before = pt_now();
    pt_sleep(pt_from_sec(0.5));
    ptime_t after = pt_now();

    PU_ASSERT(pt_to_sec(after - before) >= 0.5);

    return true;
}

PU_TEST(test_usec)
{
    PU_ASSERT(pt_to_usec(pt_from_usec(1000)) == 1000);
    PU_ASSERT(pt_to_msec(pt_from_usec(1000000)) == 1000);
    PU_ASSERT(pt_to_sec(pt_from_usec(1000000000)) == 1000.0);

    PU_ASSERT(pt_from_usec(1) == pt_from_usec(1));
    PU_ASSERT(pt_from_msec(1) == pt_from_usec(1000));
    PU_ASSERT(pt_from_sec(1.0) == pt_from_usec(1000000));

    return true;
}

PU_TEST(test_msec)
{
    PU_ASSERT(pt_to_usec(pt_from_msec(1)) == 1000);
    PU_ASSERT(pt_to_msec(pt_from_msec(1000)) == 1000);
    PU_ASSERT(pt_to_sec(pt_from_msec(1000000)) == 1000.0);

    PU_ASSERT(pt_from_usec(1000) == pt_from_msec(1));
    PU_ASSERT(pt_from_msec(1000) == pt_from_msec(1000));
    PU_ASSERT(pt_from_sec(1.0) == pt_from_msec(1000));

    return true;
}

PU_TEST(test_sec)
{
    PU_ASSERT(pt_to_usec(pt_from_sec(1.0)) == 1000000);
    PU_ASSERT(pt_to_msec(pt_from_sec(1.0)) == 1000);
    PU_ASSERT(pt_to_sec(pt_from_sec(1.0)) == 1.0);

    PU_ASSERT(pt_from_usec(1) == pt_from_sec(0.000001));
    PU_ASSERT(pt_from_msec(1) == pt_from_sec(0.001));
    PU_ASSERT(pt_from_sec(1.0) == pt_from_sec(1.0));

    return true;
}

int main (int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    pu_display_colors(true);
    PU_RUN_TEST(test_sleep);
    PU_RUN_TEST(test_usec);
    PU_RUN_TEST(test_msec);
    PU_RUN_TEST(test_sec);
    pu_print_stats();

    return pu_num_failed == 0 ? 0 : -1;
}
