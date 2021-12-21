#ifndef PICO_GL_TYPES
#define PICO_GL_TYPES

#include <stdint.h>

typedef int8_t   pgl_int8_t;
typedef uint8_t  pgl_uint8_t;
typedef int16_t  pgl_int16_t;
typedef uint16_t pgl_uint16_t;
typedef int32_t  pgl_int32_t;
typedef uint32_t pgl_uint32_t;
typedef int64_t  pgl_int64_t;
typedef uint64_t pgl_uint64_t;

#ifdef _WIN64
typedef signed   long long int pgl_intptr_t;
typedef unsigned long long int pgl_uintptr_t;
typedef signed   long long int pgl_ssize_t;
typedef unsigned long long int pgl_usize_t;
#else
typedef signed   long  int pgl_intptr_t;
typedef unsigned long  int pgl_uintptr_t;
typedef signed   long  int pgl_ssize_t;
typedef unsigned long  int pgl_usize_t;
#endif

typedef float pgl_float_t;

#endif // PICO_GL_TYPES
