/*
    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Summary:
    --------
    A simple Base64 encoding/decoding library. This library is a repackaged
    version of the [b64.c](https://github.com/littlstar/b64.c) library into a
    single header format. Aside from a few formatting modifications some
    comments and macros, the code is largely true to the original.
*/

#ifndef PICO_B64_H
#define PICO_B64_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

char* b64_encode(const unsigned char* buf, size_t size);

unsigned char* b64_decode(const char* buf, size_t size);

unsigned char* b64_decode_ex(const char* buf, size_t in_size, size_t* out_size);

#ifdef __cplusplus
}
#endif

#endif // PICO_B64_H

#ifdef PICO_B64_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef PICO_B64_BUFFER_SIZE
#define PICO_B64_BUFFER_SIZE (1024 * 64) // 64K
#endif

#if !defined(PICO_B64_MALLOC) || !defined(PICO_B64_REALLOC)
#include <stdlib.h>
#define PICO_B64_MALLOC(size)       (malloc(size))
#define PICO_B64_REALLOC(ptr, size) (realloc(ptr, size))
#endif

/*=============================================================================
 * Internal aliases
 *============================================================================*/
#define B64_MALLOC      PICO_B64_MALLOC
#define B64_REALLOC     PICO_B64_REALLOC
#define B64_BUFFER_SIZE PICO_B64_BUFFER_SIZE

static const char b64_table[] =
{
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/'
};

static void* b64_buf_init(size_t* bufc);
static void* b64_buf_resize(void* ptr, size_t size, size_t* bufc);

static void* b64_buf_init(size_t* bufc) {
    void* buf = B64_MALLOC(B64_BUFFER_SIZE);
    *bufc = 1;
    return buf;
}

void* b64_buf_resize(void* ptr, size_t size, size_t* bufc) {
    if (size > (*bufc) * B64_BUFFER_SIZE) {

        while (size > (*bufc) * B64_BUFFER_SIZE)
            (*bufc)++;

        void* buf = B64_REALLOC(ptr, B64_BUFFER_SIZE * (*bufc));

        if (!buf)
            return NULL;
        else
            return buf;
    }

    return ptr;
}


#endif // PICO_B64_IMPLEMENTATION

/*
The MIT License (MIT)

Copyright (c) 2014 Little Star Media, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
