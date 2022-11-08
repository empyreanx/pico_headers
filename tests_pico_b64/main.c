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

TEST_CASE(test_encode)
{
    REQUIRE(encode_test("Many hands make light work.",
                          "TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu"));

    REQUIRE(encode_test("light work.", "bGlnaHQgd29yay4="));
    REQUIRE(encode_test("light work" , "bGlnaHQgd29yaw=="));
    REQUIRE(encode_test("light wor"  , "bGlnaHQgd29y"));
    REQUIRE(encode_test("light wo"   , "bGlnaHQgd28="));
    REQUIRE(encode_test("light w"    , "bGlnaHQgdw=="));

    REQUIRE(encode_test(""       , ""));
    REQUIRE(encode_test("f"      , "Zg=="));
    REQUIRE(encode_test("fo"     , "Zm8="));
    REQUIRE(encode_test("foo"    , "Zm9v"));
    REQUIRE(encode_test("foob"   , "Zm9vYg=="));
    REQUIRE(encode_test("fooba"  , "Zm9vYmE="));
    REQUIRE(encode_test("foobar" , "Zm9vYmFy"));
    REQUIRE(encode_test("a+b/c"  , "YStiL2M="));

    return true;
}

TEST_CASE(test_decode)
{
    REQUIRE(decode_test("", ""));

    REQUIRE(decode_test("TWFueSBoYW5kcyBtYWtlIGxpZ2h0IHdvcmsu",
                          "Many hands make light work."));

    REQUIRE(decode_test("bGlnaHQgd29yay4=", "light work."));
    REQUIRE(decode_test("bGlnaHQgd29yaw==", "light work"));
    REQUIRE(decode_test("bGlnaHQgd29y"    , "light wor"));
    REQUIRE(decode_test("bGlnaHQgd28="    , "light wo"));
    REQUIRE(decode_test("bGlnaHQgdw=="    , "light w"));

    REQUIRE(decode_test(""        , ""));
    REQUIRE(decode_test("Zg=="    , "f"));
    REQUIRE(decode_test("Zm8="    , "fo"));
    REQUIRE(decode_test("Zm9v"    , "foo"));
    REQUIRE(decode_test("Zm9vYg==", "foob"));
    REQUIRE(decode_test("Zm9vYmE=", "fooba"));
    REQUIRE(decode_test("Zm9vYmFy", "foobar"));
    REQUIRE(decode_test("YStiL2M=", "a+b/c"));

    return true;
}

int main()
{
    pu_display_colors(true);
    RUN_TEST_CASE(test_encode);
    RUN_TEST_CASE(test_decode);
    pu_print_stats();
    return pu_test_failed();
}

