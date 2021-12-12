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

/**
 * For a more complete demonstration, see example2.c
 */

#define PUNIT_IMPLEMENTATION
#include "../pico_unit.h"

#include <string.h>  /* strcmp */

/*
 * Used to extend PU_ASSERT
 */
static bool str_eq(const char* str1, const char* str2)
{
    return 0 == strcmp(str1, str2);
}

/*
 * Passing test. Note that the test function declaration returns a boolean value
 * and that the test definition returns true. All test functions must return
 * true.
 */

PU_TEST(test1)
{
    PU_ASSERT(2 + 2 == 4);                /* Boolean assertion (ok)         */
    PU_ASSERT(str_eq("apples", "apples"));  /* String equality assertion (ok) */
    return true;
}

/* Failing test */
PU_TEST(test2)
{
    PU_ASSERT(2 + 2 != 4);                /* Boolean assertion (fails) */
    PU_ASSERT(str_eq("apples", "oranges")); /* String equality (fails */
    return true;
}

/* Mixed results */
PU_TEST(test3)
{
    PU_ASSERT(2 + 2 == 4);                 /* Boolean assertion (ok) */
    PU_ASSERT(str_eq("apples", "oranges"));  /* String equality fails */
    return true;
}

/* Test suite container function (multiple test suites can be specified. */
static PU_SUITE(test_suite)
{
    PU_RUN_TEST(test1);
    PU_RUN_TEST(test2);
    PU_RUN_TEST(test3);
}

int
main ()
{
    PU_RUN_SUITE(test_suite);
    pu_print_stats(); /* Optional */
    return 0;
}

/* EoF */
