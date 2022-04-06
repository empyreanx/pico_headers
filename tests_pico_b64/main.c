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
    PU_ASSERT(encode_test("Many hands make light work.",
                          "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu"));

    PU_ASSERT(encode_test("light work.", "bGlnaHQgd29yay4="));
    PU_ASSERT(encode_test("light work" , "bGlnaHQgd29yaw=="));
    PU_ASSERT(encode_test("light wor"  , "bGlnaHQgd29y"));
    PU_ASSERT(encode_test("light wo"   , "bGlnaHQgd28="));
    PU_ASSERT(encode_test("light w"    , "bGlnaHQgdw=="));

    PU_ASSERT(encode_test(""       , ""));
    PU_ASSERT(encode_test("f"      , "Zg=="));
    PU_ASSERT(encode_test("fo"     , "Zm8="));
    PU_ASSERT(encode_test("foo"    , "Zm9v"));
    PU_ASSERT(encode_test("foob"   , "Zm9vYg=="));
    PU_ASSERT(encode_test("fooba"  , "Zm9vYmE="));
    PU_ASSERT(encode_test("foobar" , "Zm9vYmFy"));
    PU_ASSERT(encode_test("a+b/c"  , "YStiL2M="));

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
    PU_ASSERT(decode_test("bGlnaHQgd28="    , "light wo"));
    PU_ASSERT(decode_test("bGlnaHQgdw=="    , "light w"));

    PU_ASSERT(decode_test(""        , ""));
    PU_ASSERT(decode_test("Zg=="    , "f"));
    PU_ASSERT(decode_test("Zm8="    , "fo"));
    PU_ASSERT(decode_test("Zm9v"    , "foo"));
    PU_ASSERT(decode_test("Zm9vYg==", "foob"));
    PU_ASSERT(decode_test("Zm9vYmE=", "fooba"));
    PU_ASSERT(decode_test("Zm9vYmFy", "foobar"));
    PU_ASSERT(decode_test("YStiL2M=", "a+b/c"));

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

