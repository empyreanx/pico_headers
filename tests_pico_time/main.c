#define PICO_TIME_IMPLEMENTATION
#include "../pico_time.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

TEST_CASE(test_sleep)
{
    ptime_t before = pt_now();
    pt_sleep(pt_from_sec(0.5));
    ptime_t after = pt_now();

    REQUIRE(pt_to_sec(after - before) >= 0.5);

    return true;
}

TEST_CASE(test_usec)
{
    REQUIRE(pt_to_usec(pt_from_usec(1000)) == 1000);
    REQUIRE(pt_to_msec(pt_from_usec(1000000)) == 1000);
    REQUIRE(pt_to_sec(pt_from_usec(1000000000)) == 1000.0);

    REQUIRE(pt_from_usec(1) == pt_from_usec(1));
    REQUIRE(pt_from_msec(1) == pt_from_usec(1000));
    REQUIRE(pt_from_sec(1.0) == pt_from_usec(1000000));

    return true;
}

TEST_CASE(test_msec)
{
    REQUIRE(pt_to_usec(pt_from_msec(1)) == 1000);
    REQUIRE(pt_to_msec(pt_from_msec(1000)) == 1000);
    REQUIRE(pt_to_sec(pt_from_msec(1000000)) == 1000.0);

    REQUIRE(pt_from_usec(1000) == pt_from_msec(1));
    REQUIRE(pt_from_msec(1000) == pt_from_msec(1000));
    REQUIRE(pt_from_sec(1.0) == pt_from_msec(1000));

    return true;
}

TEST_CASE(test_sec)
{
    REQUIRE(pt_to_usec(pt_from_sec(1.0)) == 1000000);
    REQUIRE(pt_to_msec(pt_from_sec(1.0)) == 1000);
    REQUIRE(pt_to_sec(pt_from_sec(1.0)) == 1.0);

    REQUIRE(pt_from_usec(1) == pt_from_sec(0.000001));
    REQUIRE(pt_from_msec(1) == pt_from_sec(0.001));
    REQUIRE(pt_from_sec(1.0) == pt_from_sec(1.0));

    return true;
}

int main (int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    pu_display_colors(true);
    RUN_TEST_CASE(test_sleep);
    RUN_TEST_CASE(test_usec);
    RUN_TEST_CASE(test_msec);
    RUN_TEST_CASE(test_sec);
    pu_print_stats();

    return pu_test_failed();
}
