/**
    @file pico_emitter.h
    @brief A minimal, optimized event emitter written in C99.

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Features:
    ---------

    - Written in C99 and compatible with C++
    - Single header library for easy integration into any build system
    - Tiny memory footprint
    - Simple and minimalistic API
    - Listeners subscribe to integer-keyed event types
    - Fire-once (em_once) listener support
    - Safe removal of listeners from within callbacks
    - Permissive licensing (zlib or public domain)

    Summary:
    --------

    An event emitter dispatches events to registered listener callbacks.
    Listeners are associated with an integer event ID in the range
    [0, num_events). Multiple listeners can be registered per event and are
    invoked in registration order when the event is emitted.

    Listeners may safely call em_on, em_once, em_off, and em_off_all from
    within a callback. Newly registered listeners take effect on the next
    emit. Removals take effect after the current emit returns.

    Example:
    --------

    Define event IDs:

    > typedef enum { EVT_JUMP, EVT_LAND, EVT_COUNT } my_event_t;

    Create an emitter, subscribe, emit, and tear down:

    > void on_jump(const void* data, void* udata)
    > {
    >     printf("jumped! value=%d\n", *(int*)data);
    > }
    >
    > emitter_t* emitter = em_create(EVT_COUNT);
    >
    > em_on(emitter, EVT_JUMP, on_jump, NULL);
    >
    > int val = 42;
    > em_emit(emitter, EVT_JUMP, &val);  // prints "jumped! value=42"
    >
    > em_free(emitter);

    Usage:
    ------

    To use this library in your project, add the following

    > #define EVENTEMITTER_IMPLEMENTATION
    > #include "pico_emitter.h"

    to a source file (once), then simply include the header normally.

    Macros:
    -------

    - PICO_EMITTER_MAX_LISTENERS (default: 8)
      Maximum number of listeners per event type. Must be defined before
      EVENTEMITTER_IMPLEMENTATION.

    - PICO_EMITTER_MALLOC(size)  (default: malloc)
    - PICO_EMITTER_FREE(ptr)     (default: free)
    - PICO_EMITTER_ASSERT(expr)  (default: assert)
      Must be defined before EVENTEMITTER_IMPLEMENTATION.
*/

#ifndef PICO_EMITTER_H
#define PICO_EMITTER_H

#include <stdbool.h> /* bool, true, false */
#include <stddef.h>  /* NULL */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Listener callback signature.
 *
 * @param data  Read-only event payload supplied by the emitter. May be NULL.
 * @param udata User data registered alongside the listener. May be NULL.
 */
typedef void (*em_listener_fn)(const void* data, void* udata);

/**
 * @brief Event emitter context (opaque).
 */
typedef struct em_emitter_s emitter_t;

/**
 * @brief Creates an event emitter that supports the given number of event types.
 *
 * Event IDs are integers in the range [0, num_events).
 *
 * @param num_events Positive number of distinct event types.
 *
 * @returns A pointer to the new emitter, or NULL if allocation failed.
 */
emitter_t* em_create(int num_events);

/**
 * @brief Destroys the emitter and frees all associated memory.
 *
 * @param emitter The emitter to destroy. Must not be NULL.
 */
void em_free(emitter_t* emitter);

/**
 * @brief Subscribes a listener to an event.
 *
 * Registering the same listener multiple times results in multiple calls
 * per emit.
 *
 * @param emitter  The emitter. Must not be NULL.
 * @param event    Event ID in [0, num_events).
 * @param listener Callback to invoke. Must not be NULL.
 * @param udata    Arbitrary pointer forwarded to the callback. May be NULL.
 */
void em_on(emitter_t* emitter, int event, em_listener_fn listener, void* udata);

/**
 * @brief Subscribes a listener that fires exactly once, then unsubscribes.
 *
 * The listener is removed before it is called so that re-registration from
 * within the callback is safe.
 *
 * @param emitter  The emitter. Must not be NULL.
 * @param event    Event ID in [0, num_events).
 * @param listener Callback to invoke. Must not be NULL.
 * @param udata    Arbitrary pointer forwarded to the callback. May be NULL.
 */
void em_once(emitter_t* emitter, int event, em_listener_fn listener, void* udata);

/**
 * @brief Unsubscribes the first listener whose function pointer matches.
 *
 * If the same function was registered multiple times, call em_off once per
 * registration to remove each occurrence individually.
 *
 * @param emitter  The emitter. Must not be NULL.
 * @param event    Event ID in [0, num_events).
 * @param listener The function pointer to remove. Must not be NULL.
 */
void em_off(emitter_t* emitter, int event, em_listener_fn listener);

/**
 * @brief Removes all listeners subscribed to an event.
 *
 * @param emitter The emitter. Must not be NULL.
 * @param event   Event ID in [0, num_events).
 */
void em_off_all(emitter_t* emitter, int event);

/**
 * @brief Emits an event, invoking all registered listeners in order.
 *
 * Listeners added from within a callback are not called in the current emit.
 * Listeners removed from within a callback are not called after removal.
 * Once-listeners are removed before being called.
 *
 * @param emitter The emitter. Must not be NULL.
 * @param event   Event ID in [0, num_events).
 * @param data    Optional event payload forwarded to each listener. May be NULL.
 */
void em_emit(emitter_t* emitter, int event, const void* data);

/**
 * @brief Returns the number of listeners currently subscribed to an event.
 *
 * @param emitter The emitter. Must not be NULL.
 * @param event   Event ID in [0, num_events).
 *
 * @returns The listener count for the given event.
 */
int em_count(const emitter_t* emitter, int event);

#ifdef __cplusplus
}
#endif

#endif /* PICO_EMITTER_H */

#ifdef EVENTEMITTER_IMPLEMENTATION

#include <stdint.h>  /* uint8_t */
#include <string.h>  /* memset */

/*
 * Configuration
 */

#ifndef PICO_EMITTER_MAX_LISTENERS
#define PICO_EMITTER_MAX_LISTENERS 8
#endif

#ifdef NDEBUG
    #define PICO_EMITTER_ASSERT(expr) ((void)0)
#else
    #ifndef PICO_EMITTER_ASSERT
        #include <assert.h>
        #define PICO_EMITTER_ASSERT(expr) assert(expr)
    #endif
#endif

#ifndef PICO_EMITTER_MALLOC
    #include <stdlib.h>
    #define PICO_EMITTER_MALLOC(size) malloc(size)
    #define PICO_EMITTER_FREE(ptr)    free(ptr)
#endif

/*
 * Internal aliases
 */

#define EM_MAX_LISTENERS PICO_EMITTER_MAX_LISTENERS
#define EM_ASSERT        PICO_EMITTER_ASSERT
#define EM_MALLOC        PICO_EMITTER_MALLOC
#define EM_FREE          PICO_EMITTER_FREE

/*
 * Per-event listener slot. Uses a struct-of-arrays layout so that function
 * pointers are contiguous in memory, improving cache performance during emit.
 */
typedef struct
{
    em_listener_fn listeners[EM_MAX_LISTENERS]; /* function pointers         */
    void*          udatas[EM_MAX_LISTENERS];    /* corresponding user data   */
    uint8_t        once[EM_MAX_LISTENERS];      /* 1 = fire-once flag        */
    int            count;                       /* active listener count     */
    bool           emitting;                    /* true while iterating      */
} em_slot_t;

struct em_emitter_s
{
    em_slot_t* events;
    int        num_events;
};

/*
 * Removes NULL-tombstoned entries from a slot after emit or em_off.
 */
static void em_compact(em_slot_t* slot)
{
    int dst = 0;

    for (int src = 0; src < slot->count; src++)
    {
        if (slot->listeners[src] != NULL)
        {
            slot->listeners[dst] = slot->listeners[src];
            slot->udatas[dst]    = slot->udatas[src];
            slot->once[dst]      = slot->once[src];
            dst++;
        }
    }

    slot->count = dst;
}

/*
 * Internal helper for both em_on and em_once.
 */
static void em_subscribe(emitter_t* emitter, int event,
                         em_listener_fn listener, void* udata,
                         uint8_t once)
{
    EM_ASSERT(emitter  != NULL);
    EM_ASSERT(event    >= 0 && event < emitter->num_events);
    EM_ASSERT(listener != NULL);

    em_slot_t* slot = &emitter->events[event];

    EM_ASSERT(slot->count < EM_MAX_LISTENERS);

    slot->listeners[slot->count] = listener;
    slot->udatas[slot->count]    = udata;
    slot->once[slot->count]      = once;
    slot->count++;
}

emitter_t* em_create(int num_events)
{
    EM_ASSERT(num_events > 0);

    emitter_t* emitter = (emitter_t*)EM_MALLOC(sizeof(emitter_t));

    if (!emitter)
    {
        return NULL;
    }

    emitter->events = (em_slot_t*)EM_MALLOC((size_t)num_events * sizeof(em_slot_t));

    if (!emitter->events)
    {
        EM_FREE(emitter);
        return NULL;
    }

    memset(emitter->events, 0, (size_t)num_events * sizeof(em_slot_t));

    emitter->num_events = num_events;

    return emitter;
}

void em_free(emitter_t* emitter)
{
    EM_ASSERT(emitter != NULL);

    EM_FREE(emitter->events);
    EM_FREE(emitter);
}

void em_on(emitter_t* emitter, int event, em_listener_fn listener, void* udata)
{
    em_subscribe(emitter, event, listener, udata, 0);
}

void em_once(emitter_t* emitter, int event, em_listener_fn listener, void* udata)
{
    em_subscribe(emitter, event, listener, udata, 1);
}

void em_off(emitter_t* emitter, int event, em_listener_fn listener)
{
    EM_ASSERT(emitter  != NULL);
    EM_ASSERT(event    >= 0 && event < emitter->num_events);
    EM_ASSERT(listener != NULL);

    em_slot_t* slot = &emitter->events[event];

    for (int i = 0; i < slot->count; i++)
    {
        if (slot->listeners[i] != listener)
        {
            continue;
        }

        if (slot->emitting)
        {
            /* Tombstone; em_emit will compact after iteration. */
            slot->listeners[i] = NULL;
        }
        else
        {
            /* Compact in-place immediately. */
            for (int j = i; j < slot->count - 1; j++)
            {
                slot->listeners[j] = slot->listeners[j + 1];
                slot->udatas[j]    = slot->udatas[j + 1];
                slot->once[j]      = slot->once[j + 1];
            }

            slot->count--;
        }

        return; /* Remove first match only. */
    }
}

void em_off_all(emitter_t* emitter, int event)
{
    EM_ASSERT(emitter != NULL);
    EM_ASSERT(event   >= 0 && event < emitter->num_events);

    em_slot_t* slot = &emitter->events[event];

    if (slot->emitting)
    {
        /* Tombstone all; em_emit will compact after iteration. */
        for (int i = 0; i < slot->count; i++)
        {
            slot->listeners[i] = NULL;
        }
    }
    else
    {
        slot->count = 0;
    }
}

void em_emit(emitter_t* emitter, int event, const void* data)
{
    EM_ASSERT(emitter != NULL);
    EM_ASSERT(event   >= 0 && event < emitter->num_events);

    em_slot_t* slot = &emitter->events[event];

    /*
     * Capture the count before iterating so that listeners added during emit
     * are not called in the current dispatch.
     */
    int  count         = slot->count;
    bool needs_compact = false;

    slot->emitting = true;

    for (int i = 0; i < count; i++)
    {
        em_listener_fn fn = slot->listeners[i];

        if (!fn)
        {
            /* Tombstoned by em_off or em_off_all from a prior callback. */
            needs_compact = true;
            continue;
        }

        /*
         * Tombstone once-listeners before calling so that re-registration
         * from within the callback is safe.
         */
        if (slot->once[i])
        {
            slot->listeners[i] = NULL;
            needs_compact = true;
        }

        fn(data, slot->udatas[i]);

        /*
         * The listener may have tombstoned itself (or a later slot) via
         * em_off during the callback. Check whether this slot became NULL
         * after the call.
         */
        if (slot->listeners[i] == NULL)
        {
            needs_compact = true;
        }
    }

    slot->emitting = false;

    if (needs_compact)
    {
        em_compact(slot);
    }
}

int em_count(const emitter_t* emitter, int event)
{
    EM_ASSERT(emitter != NULL);
    EM_ASSERT(event   >= 0 && event < emitter->num_events);

    return emitter->events[event].count;
}

#endif /* EVENTEMITTER_IMPLEMENTATION */

/*
    ----------------------------------------------------------------------------
    This software is available under two licenses (A) or (B). You may choose
    either one as you wish:
    ----------------------------------------------------------------------------

    (A) The zlib License

    Copyright (c) 2026 James McLean

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

// EoF
