/**
    @file pico_time.h
    @brief A simple time management library
*/

#ifndef PICO_TIME_H
#define PICO_TIME_H

#define _POSIX_C_SOURCE 199309L

#include <stdint.h>

typedef uint64_t ptime_t;

ptime_t pt_now();
void pt_sleep(ptime_t duration);

int64_t pt_to_usec(ptime_t time);
int32_t pt_to_msec(ptime_t time);
double  pt_to_sec(ptime_t time);

ptime_t pt_from_usec(int64_t usec);
ptime_t pt_from_msec(int32_t msec);
ptime_t pt_from_sec(double sec);

#endif // PICO_TIME_H

#ifdef PT_IMPLEMENTATION

#define PT_WINDOWS 1
#define PT_MAC     2
#define PT_UNIX    3

#if defined(_WIN32)
	#define PT_PLATFORM PT_WINDOWS
#elif defined(__APPLE__)
	#define PT_PLATFORM PT_MAC
#else
	#define PT_PLATFORM PT_UNIX

#endif

#if PT_PLATFORM == PT_WINDOWS



#elif PT_PLATFORM == PT_MACOS


#elif PT_PLATFORM == PT_UNIX

#include <errno.h>
#include <time.h>

ptime_t pt_now()
{
    struct timespec ti;
    clock_gettime(CLOCK_MONOTONIC, &ti);
    return ti.tv_sec * 1000000UL + ti.tv_nsec / 1000;
}

void pt_sleep(ptime_t duration)
{
    struct timespec ti;
    ti.tv_sec = duration / 1000000;
    ti.tv_nsec = (duration % 1000000) * 1000;

    while ((nanosleep(&ti, &ti) == -1) && (errno == EINTR));
}

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
    return (ptime_t)(sec * 1000000.0);
}

#endif // PT_PLATFORM

#endif // PT_IMPLEMENTATION

