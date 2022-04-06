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

size_t b64_encoded_size(size_t len);
size_t b64_decoded_size(const char* src, size_t len);

size_t b64_encode(char* dst, const unsigned char* src, size_t len);
size_t b64_decode(unsigned char* dst, const char* src, size_t len);

#ifdef __cplusplus
}
#endif

#endif // PICO_B64_H

#ifdef PICO_B64_IMPLEMENTATION

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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
 * Buffer size functions
 *============================================================================*/

size_t b64_encoded_size(size_t len)
{
    return 4 * ceil(len / 3.0);
}

size_t b64_decoded_size(const char* src, size_t len)
{
    if (len % 4 != 0)
        return 0; // Input must be padded

    size_t padding = 0;

    //if ('=' == src[len - 3])
    //    padding = 3;

    if ('=' == src[len - 2])
        padding = 2;
    else if ('=' == src[len - 1])
        padding = 1;

    return floor(3.0 * (len - padding) / 4.0);
}

/*=============================================================================
 * Buffer encoding/decoding functions
 *============================================================================*/
static inline void b64_encode_tmp(unsigned char* buf, unsigned char* tmp)
{
    buf[0] = (tmp[0] & 0xfc) >> 2;
    buf[1] = ((tmp[0] & 0x03) << 4) + ((tmp[1] & 0xf0) >> 4);
    buf[2] = ((tmp[1] & 0x0f) << 2) + ((tmp[2] & 0xc0) >> 6);
    buf[3] = tmp[2] & 0x3f;
}

static inline void b64_decode_tmp(unsigned char* buf, unsigned char* tmp)
{
    buf[0] = (tmp[0] << 2) + ((tmp[1] & 0x30) >> 4);
    buf[1] = ((tmp[1] & 0xf) << 4) + ((tmp[2] & 0x3c) >> 2);
    buf[2] = ((tmp[2] & 0x3) << 6) + tmp[3];
}

/*=============================================================================
 * Encoding
 *============================================================================*/

size_t b64_encode(char* dst, const unsigned char* src, size_t len)
{
    int i = 0;
    int j = 0;
    size_t size = 0;
    unsigned char buf[4];
    unsigned char tmp[3];

    // parse until end of source
    while (len--)
    {
        // read up to 3 bytes at a time into `tmp'
        tmp[i++] = *(src++);

        // if 3 bytes read then encode into `buf'
        if (3 == i)
        {
            b64_encode_tmp(buf, tmp);

            // allocate 4 new byts for `enc` and
            // then translate each encoded buffer
            // part by index from the base 64 index table
            // into `enc' unsigned char array
            for (i = 0; i < 4; ++i)
            {
                dst[size++] = b64_table[buf[i]];
            }

            // reset index
            i = 0;
        }
    }

    // remainder
    if (i > 0)
    {
        // fill `tmp' with `\0' at most 3 times
        for (j = i; j < 3; ++j)
        {
            tmp[j] = '\0';
        }

        // perform same codec as above
        b64_encode_tmp(buf, tmp);

        // perform same write to `enc` with new allocation
        for (j = 0; j < i + 1; ++j)
        {
            dst[size++] = b64_table[buf[j]];
        }

        // while there is still a remainder
        // append `=' to `dst'
        while (i++ < 3)
        {
            dst[size++] = '=';
        }
    }

    return size;
}

/*=============================================================================
 * Decoding
 *============================================================================*/

static inline size_t b64_table_lookup(char symbol)
{
    size_t i;

    for (i = 0; i < 64; ++i)
    {
        if (symbol == b64_table[i])
            return i;
    }

    return 0;
}

size_t b64_decode(unsigned char* dst, const char * src, size_t len)
{
    int i = 0;
    int j = 0;
    size_t size = 0;
    unsigned char buf[3];
    unsigned char tmp[4];

    // parse until end of source
    while (len--) {
        // break if char is `=' or not base64 char
        if ('=' == src[j])
            break;

        if (!isalnum((int)src[j]) && '+' != src[j] && '/' != src[j])
            break;

        // read up to 4 bytes at a time into `tmp'
        tmp[i++] = src[j++];

        // if 4 bytes read then decode into `buf'
        if (4 == i)
        {
            // translate values in `tmp' from table
            for (i = 0; i < 4; ++i)
            {
                tmp[i] = b64_table_lookup(tmp[i]);

                // find translation char in `b64_table'
                /*for (l = 0; l < 64; ++l) {
                    if (tmp[i] == b64_table[l]) {
                        tmp[i] = l;
                        break;
                    }
                }*/
            }

            // decode
            b64_decode_tmp(buf, tmp);

            for (i = 0; i < 3; ++i)
            {
                dst[size++] = buf[i];
            }

            // reset
            i = 0;
        }
    }

    // remainder
    if (i > 0)
    {
        // fill `tmp' with `\0' at most 4 times
        for (j = i; j < 4; ++j)
        {
            tmp[j] = '\0';
        }

        // translate remainder
        for (j = 0; j < 4; ++j)
        {
            tmp[j] = b64_table_lookup(tmp[j]);
            // find translation char in `b64_table'
            /*for (l = 0; l < 64; ++l) {
                if (tmp[j] == b64_table[l]) {
                    tmp[j] = l;
                    break;
                }
            }*/
        }

        // decode remainder
        b64_decode_tmp(buf, tmp);

        for (j = 0; j < i - 1; ++j)
        {
            dst[size++] = buf[j];
        }
    }

    return size;
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
