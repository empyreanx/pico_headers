#ifndef PICO_FONTSTASH_H
#define PICO_FONTSTASH_H

#include "fontstash.h"

FONScontext* pfons_create(int atlas_w, int atlas_h);
void sfons_destroy(FONScontext* ctx);
void pfons_flush(FONScontext* ctx);

#endif // PICO_FONTSTASH_H

#ifdef PICO_FONTSTASH_IMPLEMENTATION

#include <stddef.h>

FONScontext* pfons_create(int atlas_w, int atlas_h)
{
    return NULL;
}

void sfons_destroy(FONScontext* ctx)
{

}

void pfons_flush(FONScontext* ctx)
{

}

#endif // PICO_FONTSTASH_IMPLEMENTATION

// EoF
