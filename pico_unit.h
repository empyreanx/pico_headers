/** @file picounit.h
 * picounit is a minimal, yet powerful unit testing framework written in C99.
 */

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

#ifndef PICO_UNIT_H
#define PICO_UNIT_H

#include <stdbool.h> /* bool, true, false */
#include <stddef.h>  /* NULL */
#include <string.h>  /* strcmp */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Defines a unit test.
 *
 * @param name The name of the test. Must be a valid C function name
 */
#define PU_TEST(name) static bool name()

/**
 * Asserts that the given expression evaluates to `true`. If the expression
 * evalutes to `false`, execution of the current test aborts and an error
 * message is displayed.
 *
 * @param expr The expression to evaluate
 */
#define PU_ASSERT(expr) \
    do  { \
        if (!pu_assert((expr) ? true : false, (#expr), __FILE__, __LINE__)) \
            return false; \
    } while(false)

/**
 * Asserts that the given strings are equal. If the strings are not equal,
 * execution of the enclosing test aborts and an error message is displayed.
 *
 * @param str1 A string for comparison
 * @param str2 A string for comparison
 */
#define PU_ASSERT_STREQ(str1, str2) \
    do  { \
        if (!pu_assert(0 == strcmp(str1, str2), #str1 "==" #str2, __FILE__, __LINE__)) \
            return false; \
    } while(false)

/**
 * Runs a unit test function. IMPORTANT: The function `fp_test` must return
 * `true`. The test function has the signature, `bool test_func(void)`.
 *
 * @param fp_test The test function to execute
 */
#define PU_RUN_TEST(test_fp) (pu_run_test(#test_fp, test_fp))

/**
 * Declares a test suite
 */
#define PU_SUITE(name) void name()

/**
 * Runs a series of unit tests. The test suite function has the signature,
 * `void suite_func(void)`.
 *
 * @param fp_suite The test suite function to run
 */
#define PU_RUN_SUITE(suite_fp) pu_run_suite(#suite_fp, suite_fp)

/**
 * Functions that are run before or after a number of unit tests execute.
 */
typedef void (*pu_setup_fn)(void);

/**
 * Sets the current setup and teardown functions. The setup function is called
 * prior to each unit test and the teardown function after. Either of these
 * functions can be `NULL`. The setup and teardown functions have the signature,
 * `void func(void)`.
 *
 * @param fp_setup The setup function
 * @param fp_teardown The teardown function
 *
 */
void pu_setup(pu_setup_fn fp_setup, pu_setup_fn fp_teardown);

/**
 * Disables the setup and teardown functions by setting them to `NULL`.
 */
void pu_clear_setup(void);

/**
 * Turns on terminal colors. NOTE: Off by default.
 */
void pu_display_colors(bool enabled);

/**
 * Turns on time measurement. NOTE: Off by default.
 */
void pu_display_time(bool enabled);

/**
 * Prints test statistics.
 */
void pu_print_stats(void);

/*
 * WARNING: These functions are not meant to be called directly. Use the macros
 * instead.
 */
typedef bool (*pu_test_fn)(void);
typedef void (*pu_suite_fn)(void);

bool pu_assert(bool b_expr,
               const char* const p_expr,
               const char* const p_file,
               int line);

void pu_run_test(const char* const p_name, pu_test_fn fp_test);
void pu_run_suite(const char* const p_name, pu_suite_fn fp_suite);

#ifdef __cplusplus
}
#endif

#endif // PICO_UNIT_H

#ifdef PUNIT_IMPLEMENTATION

#include <stdio.h> /* printf */
#include <time.h>  /* clock_t, clock */

#define TERM_COLOR_CODE   0x1B
#define TERM_COLOR_RED   "[1;31m"
#define TERM_COLOR_GREEN "[1;32m"
#define TERM_COLOR_BOLD  "[1m"
#define TERM_COLOR_RESET "[0m"

static unsigned pu_num_asserts  = 0;
static unsigned pu_num_passed   = 0;
static unsigned pu_num_failed   = 0;
static unsigned pu_num_suites   = 0;
static bool     pu_colors      = false;
static bool     pu_time        = false;

static pu_setup_fn pu_setup_fp    = NULL;
static pu_setup_fn pu_teardown_fp = NULL;

void
pu_setup (pu_setup_fn fp_setup, pu_setup_fn fp_teardown)
{
    pu_setup_fp = fp_setup;
    pu_teardown_fp = fp_teardown;
}

void
pu_clear_setup (void)
{
    pu_setup_fp = NULL;
    pu_teardown_fp = NULL;
}

void
pu_display_colors (bool enabled)
{
    pu_colors = enabled;
}

void
pu_display_time (bool enabled)
{
    pu_time = enabled;
}

bool
pu_assert (bool passed,
           const char* const expr,
           const char* const file,
           int line)
{
    pu_num_asserts++;

    if (passed)
    {
        return true;
    }

    if (pu_colors)
    {
        printf("(%c%sFAILED%c%s: %s (%d): %s)\n",
               TERM_COLOR_CODE, TERM_COLOR_RED,
               TERM_COLOR_CODE, TERM_COLOR_RESET,
               file, line, expr);
    }
    else
    {
        printf("(FAILED: %s (%d): %s)\n", file, line, expr);
    }

    return false;
}

void
pu_run_test (const char* const name, pu_test_fn test_fp)
{
    if (NULL != pu_setup_fp)
    {
        pu_setup_fp();
    }

    printf("Running: %s ", name);

    clock_t start_time = 0;
    clock_t end_time = 0;

    if (pu_time)
    {
        start_time = clock();
    }

    if (!test_fp())
    {
        pu_num_failed++;

        if (NULL != pu_teardown_fp)
        {
            pu_teardown_fp();
        }

        return;
    }

    if (pu_time)
    {
        end_time = clock();
    }

    if (pu_colors)
    {
        printf("(%c%sOK%c%s)", TERM_COLOR_CODE, TERM_COLOR_GREEN,
                               TERM_COLOR_CODE, TERM_COLOR_RESET);
    }
    else
    {
        printf("(OK)");
    }

    if (pu_time)
    {
        printf(" (%f secs)", (double)(end_time - start_time) / CLOCKS_PER_SEC);
    }

    printf("\n");

    pu_num_passed++;

    if (NULL != pu_teardown_fp)
    {
        pu_teardown_fp();
    }
}

void
pu_run_suite (const char* const name, pu_suite_fn suite_fp)
{
    printf("===============================================================\n");

    if (pu_colors)
    {
        printf("%c%sRunning: %s%c%s\n", TERM_COLOR_CODE, TERM_COLOR_BOLD,
                                        name,
                                        TERM_COLOR_CODE, TERM_COLOR_RESET);
    }
    else
    {
        printf("Running: %s\n", name);
    }

    printf("---------------------------------------------------------------\n");
    suite_fp();
    pu_num_suites++;
}

void
pu_print_stats (void)
{
    printf("===============================================================\n");

    if (pu_colors)
    {
        printf("Summary: Passed: %c%s%u%c%s "
               "Failed: %c%s%u%c%s "
               "Total: %u Suites: %u "
               "Asserts: %u\n",
                TERM_COLOR_CODE, TERM_COLOR_GREEN, pu_num_passed,
                TERM_COLOR_CODE, TERM_COLOR_RESET,
                TERM_COLOR_CODE, TERM_COLOR_RED, pu_num_failed,
                TERM_COLOR_CODE, TERM_COLOR_RESET,
                pu_num_passed + pu_num_failed,
                pu_num_suites,  pu_num_asserts);
    }
    else
    {
        printf("Summary: Passed: %u "
               "Failed: %u "
               "Total: %u Suites: %u "
               "Asserts: %u\n",
                pu_num_passed,
                pu_num_failed,
                pu_num_passed + pu_num_failed,
                pu_num_suites,  pu_num_asserts);
    }
}


#endif // PUNIT_IMPLEMENTATION

/* EoF */
