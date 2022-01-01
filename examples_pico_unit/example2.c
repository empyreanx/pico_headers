#define PU_IMPLEMENTATION
#include "../pico_unit.h"

/*
 * Used to extend PU_ASSERT
 */
static bool str_eq(const char* str1, const char* str2)
{
    return 0 == strcmp(str1, str2);
}

static unsigned g_fix = 0;

/* Sets up fixture for (called before test). */
static void
test_setup ()
{
    g_fix = 42;
}

/* Resets fixture (called after test). */
static void
test_teardown ()
{
    g_fix = 0;
}

/* All assertions pass in this test. */
PU_TEST(test_passing1)
{
    PU_ASSERT(1);
    PU_ASSERT(42 == 42);
    PU_ASSERT(str_eq("towel", "towel"));

    return true;
}

/*
 * All assertions pass in this test. Checks the value of the fixture initialized
 * in the test setup function.
 */
PU_TEST(test_passing2)
{
    PU_ASSERT(42 == g_fix);
    PU_ASSERT(str_eq("frog", "frog"));

    return true;
}

/* Test containing failing assertion. */
PU_TEST(test_failing1)
{
    PU_ASSERT(1);
    PU_ASSERT(24 == 42); /* Fails here */
    PU_ASSERT(1);        /* Never called */

    return true;
}

/* Another test containing a failed assertion. */
PU_TEST(test_failing2)
{
    PU_ASSERT(str_eq("frog", "butterfly")); /* Fails here */
    PU_ASSERT(true);                        /* Never called */

    return true;
}

/* A test suite containing two passing and one failing test. */
static void
test_suite1 ()
{
    pu_setup(test_setup, test_teardown);

    PU_RUN_TEST(test_passing1);
    PU_RUN_TEST(test_passing2);
    PU_RUN_TEST(test_failing1);

    pu_clear_setup();
}

/* A test suite containing two passing and one failing test. */
static void
test_suite2 ()
{
    PU_RUN_TEST(test_passing1);
    PU_RUN_TEST(test_failing2);
    PU_RUN_TEST(test_passing1);
}

/* Run all test suites and print test statistics. */
int
main ()
{
    pu_display_colors(true);
    pu_display_time(true);
    PU_RUN_SUITE(test_suite1);
    PU_RUN_SUITE(test_suite2);
    pu_print_stats();
    return 0;
}

/* EoF */
