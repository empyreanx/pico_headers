#define PICO_B64_IMPLEMENTATION
#include "../pico_b64.h"

#define PICO_UNIT_IMPLEMENTATION
#include "../pico_unit.h"

#include <stdio.h>

int main()
{
    char *bradley = "bradley";

    char* encoded = b64_encode((unsigned char*)bradley, strlen(bradley));

    printf("%s\n", encoded);

    return 0;
}
