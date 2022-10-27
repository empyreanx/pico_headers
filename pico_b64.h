/**
    @file pico_b64.h
    @brief A simple Base64 encoding/decoding library

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Features:
    ---------
    - Written in ANSI C
    - Single header library for easy build system integration
    - No dyanmic memory allocation
    - Simple and concise API
    - Permissive license (MIT)

    Summary:
    --------
    This header is a repackaged and modified version of the
    [b64.c](https://github.com/littlstar/b64.c) library by Joseph Werle. The
    most significant change is that there is no dynamic memory allocation.
    Instead, there are functions that compute the size of encoded/decoded
    buffers in advance. Other changes are mostly cosmetic and are intended to
    make the code easier to understand.

    [Base64](https://en.wikipedia.org/wiki/Base64) is a means of encoding binary
    data as plain ASCII. Each Base64 character represents log2(64) = 6 bits,
    meaning the encoded bytes occupy more memory that the original. This
    encoding is useful in circumstances where data needs to be stored or
    transmitted, but where a binary format is not possible nor desired.
    Applications include embedding binary data in JSON/XML, as well as
    representing cryptographic certificates and signatures in plain text.

    Usage:
    ------

    To use this library in your project, add the following

    > #define PICO_B64_IMPLEMENTATION
    > #include "pico_b64.h"

    to a source file (once), then simply include the header normally.
*/

#ifndef PICO_B64_H
#define PICO_B64_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Returns the Base64 encoded size of an array of bytes (NOTE: This does
 * not include a null terminator)
 *
 * @param len The length of the array of bytes
 */
size_t b64_encoded_size(size_t len);

/**
 * @brief Returns the decoded size of a Base64 string (NOTE: This does not
 * include a null terminator)
 *
 * @param src The string to decode. This is only used to determine padding and is not
 * traversed
 * @param len The length of the encoded (`src`) string
 */
size_t b64_decoded_size(const char* src, size_t len);

/**
 * @brief Encodes an array of bytes into a Base64 encoded string (NOTE: A null
 * terminator is not appended)
 *
 * @param dst Encoded character (destination) buffer
 * @param src Byte array to be encoded
 * @param len Length of `src` in bytes
 * @returns Number of encoded characters
 */
size_t b64_encode(char* dst, const unsigned char* src, size_t len);

/**
 * @brief Decodes a Base64 encoded string into an array of bytes (NOTE: A null
 * terminator is not appended)
 *
 * @param dst Decoded byte array (destination)
 * @param src Character array to be decoded
 * @param len Length of `src` in bytes
 * @returns Number of decoded bytes
 */
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
    // Input must be padded and large enough
    if (len > 0 && len % 4 != 0)
        return 0;

    if (0 == len)
        return 0;

    size_t padding = 0;

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

    // Parse until end of source
    while (len--)
    {
        // Read up to 3 bytes at a time into 'tmp'
        tmp[i++] = *(src++);

        // If 3 bytes read then encode them
        if (3 == i)
        {
            b64_encode_tmp(buf, tmp);

            // Translate buffer
            for (i = 0; i < 4; ++i)
            {
                dst[size++] = b64_table[buf[i]];
            }

            // reset index
            i = 0;
        }
    }

    // Remainder
    if (i > 0)
    {
        // Fill 'tmp' with '\0' at most twice
        for (j = i; j < 3; ++j)
        {
            tmp[j] = '\0';
        }

        // Perform same transformation as above
        b64_encode_tmp(buf, tmp);

        // Translate buffer
        for (j = 0; j < i + 1; ++j)
        {
            dst[size++] = b64_table[buf[j]];
        }

        // While there is still a remainder append '=' to 'dst'
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

    // Parse until end of source
    while (len--) {
        // Break if char is padding ('=') or not base64 char
        if ('=' == src[j])
            break;

        // Break if char is not base64
        if (!isalnum((int)src[j]) && '+' != src[j] && '/' != src[j])
            break;

        // Read up to 4 bytes at a time into 'tmp'
        tmp[i++] = src[j++];

        // If 4 bytes read then decode into 'buf'
        if (4 == i)
        {
            // Translate values in 'tmp' from table
            for (i = 0; i < 4; ++i)
            {
                tmp[i] = b64_table_lookup(tmp[i]);
            }

            // Decode transform
            b64_decode_tmp(buf, tmp);

            // Write into result
            for (i = 0; i < 3; ++i)
            {
                dst[size++] = buf[i];
            }

            // Reset
            i = 0;
        }
    }

    // Remainder
    if (i > 0)
    {
        // Fill 'tmp' with '\0' at most 3 times
        for (j = i; j < 4; ++j)
        {
            tmp[j] = '\0';
        }

        // Translate remainder
        for (j = 0; j < 4; ++j)
        {
            // find translation char in 'b64_table'
            tmp[j] = b64_table_lookup(tmp[j]);
        }

        // Decode transform
        b64_decode_tmp(buf, tmp);

        // Write into result
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

Copyright (c) 2022 James McLean
Copyright (c) 2014 Little Star Media, Inc. (Joseph Werle)

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
