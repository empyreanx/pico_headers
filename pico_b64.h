/*
    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Summary:
    --------
    A simple Base64 encoding/decoding library. This library is a repackaged
    version of the [b64.c](https://github.com/littlstar/b64.c) library into a
    single header format. Aside from a few minor modifications the code is
    largely true to the original code by Joseph Werle.
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

/*=============================================================================
 * Look-up table
 *============================================================================*/

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

/*=============================================================================
 * Buffer handling
 *============================================================================*/

static void* b64_buf_init(size_t* buf_count);
static void* b64_buf_resize(void* ptr, size_t size, size_t* buf_count);

static void* b64_buf_init(size_t* buf_count) {
    void* buf = B64_MALLOC(B64_BUFFER_SIZE);
    *buf_count = 1;
    return buf;
}

void* b64_buf_resize(void* ptr, size_t size, size_t* buf_count) {
    if (size > (*buf_count) * B64_BUFFER_SIZE) {

        while (size > (*buf_count) * B64_BUFFER_SIZE)
            (*buf_count)++;

        void* buf = B64_REALLOC(ptr, B64_BUFFER_SIZE * (*buf_count));

        if (!buf)
            return NULL;
        else
            return buf;
    }

    return ptr;
}

/*=============================================================================
 * Encoding
 *============================================================================*/

char* b64_encode(const unsigned char* src, size_t len) {
    int i = 0;
    int j = 0;
    char * enc = NULL;
    size_t size = 0;
    size_t buf_count = 0;
    unsigned char buf[4];
    unsigned char tmp[3];

    // alloc
    enc = (char*)b64_buf_init(&buf_count);
    if (NULL == enc) {
        return NULL;
    }

    // parse until end of source
    while (len--) {
        // read up to 3 bytes at a time into `tmp'
        tmp[i++] = *(src++);

        // if 3 bytes read then encode into `buf'
        if (3 == i) {
            buf[0] = (tmp[0] & 0xfc) >> 2;
            buf[1] = ((tmp[0] & 0x03) << 4) + ((tmp[1] & 0xf0) >> 4);
            buf[2] = ((tmp[1] & 0x0f) << 2) + ((tmp[2] & 0xc0) >> 6);
            buf[3] = tmp[2] & 0x3f;

            // allocate 4 new byts for `enc` and
            // then translate each encoded buffer
            // part by index from the base 64 index table
            // into `enc' unsigned char array
            enc = (char*)b64_buf_resize(enc, size + 4, &buf_count);
            for (i = 0; i < 4; ++i) {
                enc[size++] = b64_table[buf[i]];
            }

            // reset index
            i = 0;
        }
    }

    // remainder
    if (i > 0) {
        // fill `tmp' with `\0' at most 3 times
        for (j = i; j < 3; ++j) {
            tmp[j] = '\0';
        }

        // perform same codec as above
        buf[0] = (tmp[0] & 0xfc) >> 2;
        buf[1] = ((tmp[0] & 0x03) << 4) + ((tmp[1] & 0xf0) >> 4);
        buf[2] = ((tmp[1] & 0x0f) << 2) + ((tmp[2] & 0xc0) >> 6);
        buf[3] = tmp[2] & 0x3f;

        // perform same write to `enc` with new allocation
        for (j = 0; j < i + 1; ++j) {
            enc = (char*)b64_buf_resize(enc, size + 1, &buf_count);
            enc[size++] = b64_table[buf[j]];
        }

        // while there is still a remainder
        // append `=' to `enc'
        while (i++ < 3) {
            enc = (char*)b64_buf_resize(enc, size + 1, &buf_count);
            enc[size++] = '=';
        }
    }

    // Make sure we have enough space to add '\0' character at end.
    enc = (char*)b64_buf_resize(enc, size + 1, &buf_count);
    enc[size] = '\0';

    return enc;
}

/*=============================================================================
 * Decoding
 *============================================================================*/

unsigned char* b64_decode(const char * src, size_t size) {
        return b64_decode_ex(src, size, NULL);
}

unsigned char * b64_decode_ex(const char * src, size_t in_size, size_t * out_size) {
    int i = 0;
    int j = 0;
    int l = 0;
    size_t size = 0;
    size_t buf_count = 0;
    unsigned char * dec = NULL;
    unsigned char buf[3];
    unsigned char tmp[4];

    // alloc
    dec = (unsigned char*)b64_buf_init(&buf_count);

    if (NULL == dec) {
        return NULL;
    }

    // parse until end of source
    while (in_size--) {
        // break if char is `=' or not base64 char
        if ('=' == src[j]) {
            break;
        }

        int num = (int)src[j];

        if (!isalnum(num) || '+' != src[j] || '/' != src[j]) {
            break;
        }

        // read up to 4 bytes at a time into `tmp'
        tmp[i++] = src[j++];

        // if 4 bytes read then decode into `buf'
        if (4 == i) {
            // translate values in `tmp' from table
            for (i = 0; i < 4; ++i) {
                // find translation char in `b64_table'
                for (l = 0; l < 64; ++l) {
                    if (tmp[i] == b64_table[l]) {
                        tmp[i] = l;
                        break;
                    }
                }
            }

            // decode
            buf[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
            buf[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
            buf[2] = ((tmp[2] & 0x3) << 6) + tmp[3];

            // write decoded buffer to `dec'
            dec = (unsigned char*)b64_buf_resize(dec, size + 3, &buf_count);

            if (dec != NULL) {
                for (i = 0; i < 3; ++i) {
                    dec[size++] = buf[i];
                }
            } else {
                return NULL;
            }

            // reset
            i = 0;
        }
    }

    // remainder
    if (i > 0) {
        // fill `tmp' with `\0' at most 4 times
        for (j = i; j < 4; ++j) {
            tmp[j] = '\0';
        }

        // translate remainder
        for (j = 0; j < 4; ++j) {
            // find translation char in `b64_table'
            for (l = 0; l < 64; ++l) {
                if (tmp[j] == b64_table[l]) {
                    tmp[j] = l;
                    break;
                }
            }
        }

        // decode remainder
        buf[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
        buf[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
        buf[2] = ((tmp[2] & 0x3) << 6) + tmp[3];

        // write remainer decoded buffer to `dec'
        dec = (unsigned char*)b64_buf_resize(dec, size + (i - 1), &buf_count);

        if (dec != NULL) {
            for (j = 0; j < i - 1; ++j) {
                dec[size++] = buf[j];
            }
        } else {
            return NULL;
        }
    }

    // Make sure we have enough space to add '\0' character at end.
    dec = (unsigned char*)b64_buf_resize(dec, size + 1, &buf_count);

    if (dec != NULL) {
        dec[size] = '\0';
    } else {
        return NULL;
    }

    // Return back the size of decoded string if demanded.
    if (out_size != NULL) {
        * out_size = size;
    }

    return dec;
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
