/**
    @file pico_time.h
    @brief A simple time management library

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Features:
    ---------
    - Written in C99
    - Single header library for easy build system integration
    - Cross-platform time and sleep functions
    - Time conversion functions

    Summary:
    --------

    This library provides high-res time and sleep functions, as well as unit
    conversions functions.

    Even though `ptime_t` is expressed in microseconds, it is still recommended
    that you use the `pt_to_usec` and `pt_from_usec` functions should this ever
    change.

    Usage:
    ------

    To use this library in your project, add the following

    > #define PICO_TIME_IMPLEMENTATION
    > #include "pico_time.h"

    to a source file (once).

    IMPORTANT: On POSIX systems, when defining PICO_TIME_IMPLEMENTATION, one of
    three conditions must hold:

    1) #define _POSIX_C_SOURCE 199309L must precede any header include in the file
    2) This library must be included before any other headers
    3) The build system must define `_POSIX_C_SOURCE 199309L`
*/

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif

#ifndef PICO_TIME_H
#define PICO_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Time value expressed in microseconds
 */
typedef uint64_t ptime_t;

/**
 * @brief Returns the present high-res clock time
 */
ptime_t pt_now(void);

/**
 * @brief Sleeps for at least the specified duration
 *
 * Note: On most platforms this function has microsecond resolution, except on
 * Windows where it only has millisecond resoultion.
 */
void pt_sleep(ptime_t duration);

/**
 * @brief Converts time to microseconds
 */
int64_t pt_to_usec(ptime_t time);

/**
 * @brief Converts time to milliseconds
 */
int32_t pt_to_msec(ptime_t time);

/**
 * @brief Converts time to seconds
 */
double  pt_to_sec(ptime_t time);

/**
 * @brief Make time from microseconds
 */
ptime_t pt_from_usec(int64_t usec);

/**
 * @brief Make time from miliseconds
 */
ptime_t pt_from_msec(int32_t msec);

/**
 * @brief Make time from seconds
 */
ptime_t pt_from_sec(double sec);

#ifdef __cplusplus
}
#endif

#endif // PICO_TIME_H

#ifdef PICO_TIME_IMPLEMENTATION

#define PT_WINDOWS 1
#define PT_APPLE   2
#define PT_UNIX    3

#if defined(_WIN32) || defined(_WIN64) || defined (__CYGWIN__)
    #define PT_PLATFORM PT_WINDOWS
#elif defined(__APPLE__) && defined(__MACH__)
    #define PT_PLATFORM PT_APPLE
#elif defined(__unix__)
    #define PT_PLATFORM PT_UNIX
#else
    #error "Unsupported platform"
#endif

/*==============================================================================
 * Windows (pt_now/pt_sleep)
 *============================================================================*/

#if PT_PLATFORM == PT_WINDOWS

#include <windows.h>

ptime_t pt_now(void)
{
    static LARGE_INTEGER freq = { 0 };

    if (freq.QuadPart == 0)
        QueryPerformanceFrequency(&freq);

    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);

    return (1000000UL * ticks.QuadPart) / freq.QuadPart;
}

void pt_sleep(ptime_t duration)
{
    TIMECAPS tc;
    timeGetDevCaps(&tc, sizeof(TIMECAPS));

    timeBeginPeriod(tc.wPeriodMin);
    Sleep(pt_to_msec(duration));
    timeEndPeriod(tc.wPeriodMin);
}

/*==============================================================================
 * Apple (pt_now)
 *============================================================================*/

#elif PT_PLATFORM == PT_APPLE

#include <mach/mach_time.h>

ptime_t pt_now(void)
{
    static mach_timebase_info_data_t freq = { 0, 0 };

    if (freq.denom == 0)
        mach_timebase_info(&freq);

    uint64_t nsec = (mach_absolute_time() * freq.numer) / freq.denom;
    return nsec / 1000;
}

/*==============================================================================
 * Unix (pt_now)
 *============================================================================*/

#elif PT_PLATFORM == PT_UNIX

#include <errno.h>
#include <time.h>

ptime_t pt_now(void)
{
    struct timespec ti;
    clock_gettime(CLOCK_MONOTONIC, &ti);
    return ti.tv_sec * 1000000UL + ti.tv_nsec / 1000;
}

#endif // PT_PLATFORM

/*==============================================================================
 * Unix and Apple (pt_sleep)
 *============================================================================*/

#if PT_PLATFORM == PT_UNIX || PT_PLATFORM == PT_APPLE

#include <errno.h>
#include <time.h>

void pt_sleep(ptime_t duration)
{
    struct timespec ti;
    ti.tv_sec = duration / 1000000;
    ti.tv_nsec = (duration % 1000000) * 1000;

    while ((nanosleep(&ti, &ti) == -1) && (errno == EINTR));
}

#endif // PT_PLATFORM

int64_t pt_to_usec(ptime_t time)
{
    return time;
}

int32_t pt_to_msec(ptime_t time)
{
    return time / 1000;
}

double pt_to_sec(ptime_t time)
{
    return time / 1000000.0;
}

ptime_t pt_from_usec(int64_t usec)
{
    return usec;
}

ptime_t pt_from_msec(int32_t msec)
{
    return msec * 1000;
}

ptime_t pt_from_sec(double sec)
{
    return (ptime_t)(sec * 1000000.0 + 0.5);
}

#endif // PICO_TIME_IMPLEMENTATION

/*
    ----------------------------------------------------------------------------
    This software is available under two licenses (A) or (B). You may choose
    either one as you wish:
    ----------------------------------------------------------------------------

    (A) The zlib License

    Copyright (c) 2021 James McLean

    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software in a
    product, an acknowledgment in the product documentation would be appreciated
    but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.

    ----------------------------------------------------------------------------

    (B) Public Domain (www.unlicense.org)

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or distribute
    this software, either in source code form or as a compiled binary, for any
    purpose, commercial or non-commercial, and by any means.

    In jurisdictions that recognize copyright laws, the author or authors of
    this software dedicate any and all copyright interest in the software to the
    public domain. We make this dedication for the benefit of the public at
    large and to the detriment of our heirs and successors. We intend this
    dedication to be an overt act of relinquishment in perpetuity of all present
    and future rights to this software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
    ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

