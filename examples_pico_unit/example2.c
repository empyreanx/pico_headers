/*=============================================================================
 * MIT License
 *
 * Copyright (c) 2020 James McLean
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
=============================================================================*/

#define PUNIT_IMPLEMENTATION
#include "../pico_unit.h"

static unsigned g_fix = 0;

/* Sets up fixture for (called before test). */
static void
test_setup ()
{
    g_fix = 42;
}

/* Resets fixture (called after test). */
void static
test_teardown ()
{
    g_fix = 0;
}

/* All assertions pass in this test. */
PUNIT_TEST(test_passing1)
{
    PUNIT_ASSERT(1);
    PUNIT_ASSERT(42 == 42);
    PUNIT_ASSERT_STREQ("towel", "towel");

    return true;
}

/*
 * All assertions pass in this test. Checks the value of the fixture initialized
 * in the test setup function.
 */
PUNIT_TEST(test_passing2)
{
    PUNIT_ASSERT(42 == g_fix);
    PUNIT_ASSERT_STREQ("frog", "frog");

    return true;
}

/* Test containing failing assertion. */
PUNIT_TEST(test_failing1)
{
    PUNIT_ASSERT(1);
    PUNIT_ASSERT(24 == 42); /* Fails here */
    PUNIT_ASSERT(1);        /* Never called */

    return true;
}

/* Another test containing a failed assertion. */
PUNIT_TEST(test_failing2)
{
    PUNIT_ASSERT_STREQ("frog", "butterfly"); /* Fails here */
    PUNIT_ASSERT(true);                      /* Never called */

    return true;
}

/* A test suite containing two passing and one failing test. */
static void
test_suite1 ()
{
    punit_setup(test_setup, test_teardown);

    PUNIT_RUN_TEST(test_passing1);
    PUNIT_RUN_TEST(test_passing2);
    PUNIT_RUN_TEST(test_failing1);

    punit_clear_setup();
}

/* A test suite containing two passing and one failing test. */
static void
test_suite2 ()
{
    PUNIT_RUN_TEST(test_passing1);
    PUNIT_RUN_TEST(test_failing2);
    PUNIT_RUN_TEST(test_passing1);
}

/* Run all test suites and print test statistics. */
int
main ()
{
    punit_colors_enabled(true);
    punit_time_enabled(true);
    PUNIT_RUN_SUITE(test_suite1);
    PUNIT_RUN_SUITE(test_suite2);
    punit_print_stats();
    return 0;
}

/* EoF */
