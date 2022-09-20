#include <stdio.h>

#define PICO_SAT_IMPLEMENTATION
#include "../pico_sat.h"

#define PICO_MATH_IMPLEMENTATION
#include "../pico_math.h"

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    printf("Hello SAT\n");
    return 0;
}
