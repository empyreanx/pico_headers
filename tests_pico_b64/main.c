#define PICO_B64_IMPLEMENTATION
#include "../pico_b64.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

#include <stdio.h>

int main()
{
    char *bradley = "bradley";

    size_t size = b64_encoded_size(strlen(bradley)) + 1;

    char enc_buf[size];

    b64_encode(enc_buf, (unsigned char*)bradley, size - 1);
    enc_buf[size] = '\0';

    printf("%s\n", enc_buf);

    char* str = "Y2FzaWxsZXJv";

    size = b64_decoded_size(str, strlen(str)) + 1;

    unsigned char dec_buf[size];

    b64_decode(dec_buf, str, strlen(str));
    dec_buf[size] = '\0';

    printf("%s\n", (char*)dec_buf);

    return 0;
}

