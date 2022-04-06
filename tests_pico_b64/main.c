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
    PU_ASSERT(encode_test("light work.", "bGlnaHQgd29yay4="));

    return true;
}

int main()
{
    PU_RUN_TEST(test_encode);

    char* enc = "light wor";

    size_t size = b64_encoded_size(strlen(enc)) + 1;

    char enc_buf[size];

    b64_encode(enc_buf, (unsigned char*)enc, strlen(enc));
    enc_buf[size - 1] = '\0';

    printf("%s\n", enc_buf);

    char* dec = "Y2FzaWxsZXJv";

    size = b64_decoded_size(dec, strlen(dec)) + 1;

    unsigned char dec_buf[size];

    b64_decode(dec_buf, dec, strlen(dec));
    dec_buf[size - 1] = '\0';

    printf("%s\n", (char*)dec_buf);

    return 0;
}

