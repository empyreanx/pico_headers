#define PICO_B64_IMPLEMENTATION
#include "../pico_b64.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

#include <stdio.h>

static bool encode_test(const char* src, const char* expected)
{
    size_t size = b64_encoded_size(strlen(src)) + 1;
    char buf[size];
    b64_encode(buf, (unsigned char*)src, strlen(src));
    buf[size - 1] = '\0';
    return 0 == strcmp(buf, expected);
}

static bool decode_test(const char* src, const char* expected)
{
    size_t size = b64_decoded_size(src, strlen(src)) + 1;
    unsigned char buf[size];
    b64_decode(buf, src, strlen(src));
    buf[size - 1] = '\0';
    return 0 == strcmp((char*)buf, expected);
}

PU_TEST(test_encode)
{
    PU_ASSERT(encode_test("", ""));

    PU_ASSERT(encode_test("Many hands make light work.",
                          "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu"));

    PU_ASSERT(encode_test("light work.", "bGlnaHQgd29yay4="));
    PU_ASSERT(encode_test("light work" , "bGlnaHQgd29yaw=="));
    PU_ASSERT(encode_test("light wor"  , "bGlnaHQgd29y"));

    return true;
}

PU_TEST(test_decode)
{
    PU_ASSERT(decode_test("", ""));

    PU_ASSERT(decode_test("TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu",
                          "Many hands make light work."));

    PU_ASSERT(decode_test("bGlnaHQgd29yay4=", "light work."));
    PU_ASSERT(decode_test("bGlnaHQgd29yaw==", "light work"));
    PU_ASSERT(decode_test("bGlnaHQgd29y"    , "light wor"));

    return true;
}


int main()
{
    pu_display_colors(true);
    PU_RUN_TEST(test_encode);
    PU_RUN_TEST(test_decode);
    pu_print_stats();
    return 0;
}

