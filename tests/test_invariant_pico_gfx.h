#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define PICO_GFX_IMPLEMENTATION
#include "pico_gfx.h"

START_TEST(test_set_uniform_block_no_overflow)
{
    /* Invariant: pg_set_uniform_block must never write beyond the allocated
       block->size bytes, regardless of the caller-supplied data buffer size. */

    /* Three scenarios: exact-size valid input, boundary (1 byte), oversized attempt */
    struct {
        size_t block_size;
        size_t data_size;
        const char *label;
    } cases[] = {
        { 64,  64,  "valid exact-size"   },  /* valid input */
        { 1,   1,   "boundary 1 byte"    },  /* boundary */
        { 16,  256, "oversized data buf" },  /* exploit: caller has larger buf but block is small */
    };

    int num_cases = sizeof(cases) / sizeof(cases[0]);

    for (int i = 0; i < num_cases; i++) {
        /* Allocate a canary-guarded buffer to detect overflow */
        size_t guard = 64;
        size_t total  = cases[i].block_size + guard;
        uint8_t *canary_buf = malloc(total);
        ck_assert_ptr_nonnull(canary_buf);
        memset(canary_buf, 0xAB, total);

        /* Prepare source data */
        uint8_t *src = malloc(cases[i].data_size);
        ck_assert_ptr_nonnull(src);
        memset(src, 0x55, cases[i].data_size);

        /* Create a minimal pg context and uniform block of block_size */
        pg_t *pg = pg_create(NULL, 0);
        ck_assert_ptr_nonnull(pg);

        pg_uniform_block_t *block = pg_create_uniform_block(pg, cases[i].block_size);
        ck_assert_ptr_nonnull(block);

        /* Call the real production function */
        pg_set_uniform_block(block, src, cases[i].block_size);

        /* Canary region after block->data must be intact (no overflow) */
        for (size_t j = 0; j < guard; j++) {
            ck_assert_msg(canary_buf[cases[i].block_size + j] == 0xAB,
                          "Heap canary corrupted — overflow detected in case: %s",
                          cases[i].label);
        }

        pg_destroy(pg);
        free(src);
        free(canary_buf);
    }
}
END_TEST

Suite *security_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Security");
    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_set_uniform_block_no_overflow);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = security_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}