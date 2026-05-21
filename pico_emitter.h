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
    - Fire-once (emitter_once) listener support
    - Safe removal of listeners from within callbacks
    - Permissive licensing (zlib or public domain)

    Summary:
    --------

    An event emitter dispatches events to registered listener callbacks.
    Listeners are associated with an integer event ID in the range
    [0, num_events). Multiple listeners can be registered per event and are
    invoked in registration order when the event is emitted.

    Listeners may safely call emitter_on, emitter_once, emitter_off, and emitter_off_all from
    within a callback. Newly registered listeners take effect on the next
    emit. Removals take effect after the current emit returns.

    Example:
    --------

    Define event IDs:

        typedef enum { EVT_JUMP, EVT_LAND, EVT_COUNT } my_event_t;

    Create an emitter, subscribe, emit, and tear down:

        void on_jump(const void* data, void* udata)
        {
            printf("jumped! value=%d\n", *(int*)data);
        }

        emitter_t* emitter = emitter_create(EVT_COUNT);

        emitter_on(emitter, EVT_JUMP, on_jump, NULL);

        int val = 42;
        emitter_emit(emitter, EVT_JUMP, &val);  // prints "jumped! value=42"

        emitter_destroy(emitter);

    Usage:
    ------

    To use this library in your project, add the following

        #define PICO_EMITTER_IMPLEMENTATION
        #include "pico_emitter.h"

    to a source file (once), then simply include the header normally.

    Macros:
    -------

    - PICO_EMITTER_INIT_CAPACITY (default: 8)
      Initial listener capacity per event slot. Slots grow automatically
      beyond this limit by doubling. Must be defined before
      PICO_EMITTER_IMPLEMENTATION.

    - PICO_EMITTER_MALLOC(size)        (default: malloc)
    - PICO_EMITTER_REALLOC(p,sz)       (default: realloc)
    - PICO_EMITTER_FREE(ptr)           (default: free)
    - PICO_EMITTER_MEMCPY(dst,src,n)   (default: memcpy)
    - PICO_EMITTER_ASSERT(expr)        (default: assert)
      Must be defined before PICO_EMITTER_IMPLEMENTATION.
*/

#ifndef PICO_EMITTER_H
#define PICO_EMITTER_H

#include <stdbool.h> // bool, true, false
#include <stddef.h>  // NULL

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Listener callback signature.
 *
 * @param data  Read-only event payload supplied by the emitter. May be NULL.
 * @param udata User data registered alongside the listener. May be NULL.
 */
typedef void (*emitter_listener_fn)(const void* data, void* udata);

/**
 * @brief Event emitter context (opaque).
 */
typedef struct emitter_emitter_s emitter_t;

/**
 * @brief Creates an event emitter that supports the given number of event types.
 *
 * Event IDs are integers in the range [0, num_events).
 *
 * @param num_events Positive number of distinct event types.
 *
 * @returns A pointer to the new emitter, or NULL if allocation failed.
 */
emitter_t* emitter_create(int num_events);

/**
 * @brief Destroys the emitter and frees all associated memory.
 *
 * @param emitter The emitter to destroy. Must not be NULL.
 */
void emitter_destroy(emitter_t* emitter);

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
void emitter_on(emitter_t* emitter, int event, emitter_listener_fn listener, void* udata);

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
void emitter_once(emitter_t* emitter, int event, emitter_listener_fn listener, void* udata);

/**
 * @brief Unsubscribes the first listener whose function pointer matches.
 *
 * If the same function was registered multiple times, call emitter_off once per
 * registration to remove each occurrence individually.
 *
 * @param emitter  The emitter. Must not be NULL.
 * @param event    Event ID in [0, num_events).
 * @param listener The function pointer to remove. Must not be NULL.
 */
void emitter_off(emitter_t* emitter, int event, emitter_listener_fn listener);

/**
 * @brief Removes all listeners subscribed to an event.
 *
 * @param emitter The emitter. Must not be NULL.
 * @param event   Event ID in [0, num_events).
 */
void emitter_off_all(emitter_t* emitter, int event);

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
void emitter_emit(emitter_t* emitter, int event, const void* data);

/**
 * @brief Returns the number of listeners currently subscribed to an event.
 *
 * @param emitter The emitter. Must not be NULL.
 * @param event   Event ID in [0, num_events).
 *
 * @returns The listener count for the given event.
 */
int emitter_count(const emitter_t* emitter, int event);

/* --------------------------------------------------------------------------
 * Queued emitter
 * -------------------------------------------------------------------------- */

/**
 * @brief Queued event emitter context (opaque).
 *
 * Wraps an emitter_t and stores events in a buffer instead of dispatching
 * them immediately. Buffered events are dispatched in FIFO order when
 * queued_emitter_flush is called. Subscription functions delegate directly
 * to the underlying emitter.
 */
typedef struct queued_emitter_s queued_emitter_t;

/**
 * @brief Creates a queued emitter supporting the given number of event types.
 *
 * @param num_events Positive number of distinct event types.
 *
 * @returns A pointer to the new queued emitter, or NULL if allocation failed.
 */
queued_emitter_t* queued_emitter_create(int num_events);

/**
 * @brief Destroys the queued emitter and discards any unflushed events.
 *
 * @param qe The queued emitter. Must not be NULL.
 */
void queued_emitter_destroy(queued_emitter_t* qe);

/**
 * @brief Subscribes a persistent listener to an event.
 *
 * @param qe       The queued emitter. Must not be NULL.
 * @param event    Event ID in [0, num_events).
 * @param listener Callback to invoke. Must not be NULL.
 * @param udata    Arbitrary pointer forwarded to the callback. May be NULL.
 */
void queued_emitter_on(queued_emitter_t* qe, int event, emitter_listener_fn listener, void* udata);

/**
 * @brief Subscribes a listener that fires exactly once, then unsubscribes.
 *
 * @param qe       The queued emitter. Must not be NULL.
 * @param event    Event ID in [0, num_events).
 * @param listener Callback to invoke. Must not be NULL.
 * @param udata    Arbitrary pointer forwarded to the callback. May be NULL.
 */
void queued_emitter_once(queued_emitter_t* qe, int event, emitter_listener_fn listener, void* udata);

/**
 * @brief Unsubscribes the first listener whose function pointer matches.
 *
 * @param qe       The queued emitter. Must not be NULL.
 * @param event    Event ID in [0, num_events).
 * @param listener The function pointer to remove. Must not be NULL.
 */
void queued_emitter_off(queued_emitter_t* qe, int event, emitter_listener_fn listener);

/**
 * @brief Removes all listeners subscribed to an event.
 *
 * @param qe    The queued emitter. Must not be NULL.
 * @param event Event ID in [0, num_events).
 */
void queued_emitter_off_all(queued_emitter_t* qe, int event);

/**
 * @brief Enqueues an event for deferred dispatch.
 *
 * If data is non-NULL and data_size is greater than zero, the payload is
 * copied into the emitter's internal arena. The caller does not need to keep
 * the pointed-to object alive after this call returns.
 *
 * @param qe        The queued emitter. Must not be NULL.
 * @param event     Event ID in [0, num_events).
 * @param data      Optional event payload. May be NULL.
 * @param data_size Size in bytes of the object pointed to by data. Pass 0
 *                  when data is NULL or when a pointer-only reference is
 *                  intentionally stored without copying.
 */
void queued_emitter_emit(queued_emitter_t* qe, int event, const void* data, size_t data_size);

/**
 * @brief Dispatches all queued events in FIFO order and clears the queue.
 *
 * Events enqueued by listeners during flush are deferred to the next call.
 *
 * @param qe The queued emitter. Must not be NULL.
 */
void queued_emitter_flush(queued_emitter_t* qe);

/**
 * @brief Returns the number of listeners currently subscribed to an event.
 *
 * @param qe    The queued emitter. Must not be NULL.
 * @param event Event ID in [0, num_events).
 *
 * @returns The listener count for the given event.
 */
int queued_emitter_count(const queued_emitter_t* qe, int event);

#ifdef __cplusplus
}
#endif

#endif // PICO_EMITTER_H

#ifdef PICO_EMITTER_IMPLEMENTATION

#include <stdint.h>   // uint8_t
#include <stdalign.h> // alignof
#include <string.h>   // memset

/*
 * Configuration
 */

#ifndef PICO_EMITTER_INIT_CAPACITY
#define PICO_EMITTER_INIT_CAPACITY 8
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
    #define PICO_EMITTER_MALLOC(size)    malloc(size)
    #define PICO_EMITTER_REALLOC(p, sz)  realloc(p, sz)
    #define PICO_EMITTER_FREE(ptr)       free(ptr)
#endif

#ifndef PICO_EMITTER_MEMCPY
    #define PICO_EMITTER_MEMCPY(dst, src, n) memcpy(dst, src, n)
#endif

/*
 * Internal aliases
 */

#define EMITTER_INIT_CAPACITY PICO_EMITTER_INIT_CAPACITY
#define EMITTER_ASSERT        PICO_EMITTER_ASSERT
#define EMITTER_MALLOC        PICO_EMITTER_MALLOC
#define EMITTER_REALLOC       PICO_EMITTER_REALLOC
#define EMITTER_FREE          PICO_EMITTER_FREE
#define EMITTER_MEMCPY        PICO_EMITTER_MEMCPY

/*
 * Per-event listener slot. Uses a struct-of-arrays layout so that function
 * pointers are contiguous in memory, improving cache performance during emit.
 * Arrays are heap-allocated and grow by doubling when capacity is exceeded.
 */
typedef struct
{
    emitter_listener_fn* listeners; // function pointers
    void**               udatas;    // corresponding user data
    uint8_t*             once;      // 1 = fire-once flag
    int                  count;     // active listener count
    int                  capacity;  // allocated capacity
    bool                 emitting;  // true while iterating
} emitter_slot_t;

struct emitter_emitter_s
{
    emitter_slot_t* events;
    int num_events;
};

/* --------------------------------------------------------------------------
 * Arena allocator types
 * -------------------------------------------------------------------------- */

typedef struct arena_block_s
{
    uint8_t*              memory;
    size_t                size;
    size_t                offset;
    struct arena_block_s* next;
} arena_block_t;

typedef struct arena_s
{
    arena_block_t* first;
    arena_block_t* current;
    size_t         block_size;
} arena_t;

/*
 * Grows a slot's listener arrays by doubling capacity.
 */
static void emitter_grow_slot(emitter_slot_t* slot)
{
    int new_cap = slot->capacity == 0 ? EMITTER_INIT_CAPACITY : slot->capacity * 2;

    emitter_listener_fn* l = (emitter_listener_fn*)EMITTER_REALLOC(slot->listeners,
                                                   (size_t)new_cap * sizeof(emitter_listener_fn));
    EMITTER_ASSERT(l != NULL);
    slot->listeners = l;

    void** u = (void**)EMITTER_REALLOC(slot->udatas, (size_t)new_cap * sizeof(void*));
    EMITTER_ASSERT(u != NULL);
    slot->udatas = u;

    uint8_t* o = (uint8_t*)EMITTER_REALLOC(slot->once, (size_t)new_cap * sizeof(uint8_t));
    EMITTER_ASSERT(o != NULL);
    slot->once = o;

    slot->capacity = new_cap;
}

/*
 * Removes NULL-tombstoned entries from a slot after emit or emitter_off.
 */
static void emitter_compact(emitter_slot_t* slot)
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
 * Internal helper for both emitter_on and emitter_once.
 */
static void emitter_subscribe(emitter_t* emitter, int event,
                         emitter_listener_fn listener, void* udata,
                         uint8_t once)
{
    EMITTER_ASSERT(emitter  != NULL);
    EMITTER_ASSERT(event    >= 0 && event < emitter->num_events);
    EMITTER_ASSERT(listener != NULL);

    emitter_slot_t* slot = &emitter->events[event];

    if (slot->count == slot->capacity)
    {
        emitter_grow_slot(slot);
    }

    slot->listeners[slot->count] = listener;
    slot->udatas[slot->count]    = udata;
    slot->once[slot->count]      = once;
    slot->count++;
}

/* --------------------------------------------------------------------------
 * Arena forward declarations
 * -------------------------------------------------------------------------- */

static arena_block_t* arena_block_create(size_t size);
static bool           arena_init(arena_t* arena, size_t initial_block_size);
static bool           arena_grow(arena_t* arena, size_t min_size);
static uintptr_t      arena_align_forward(uintptr_t ptr, size_t align);
static void*          arena_alloc_align(arena_t* arena, size_t size, size_t align);
static void*          arena_alloc(arena_t* arena, size_t size);
static void           arena_reset(arena_t* arena);
static void           arena_destroy(arena_t* arena);

emitter_t* emitter_create(int num_events)
{
    EMITTER_ASSERT(num_events > 0);

    emitter_t* emitter = (emitter_t*)EMITTER_MALLOC(sizeof(emitter_t));

    if (!emitter)
    {
        return NULL;
    }

    emitter->events = (emitter_slot_t*)EMITTER_MALLOC((size_t)num_events * sizeof(emitter_slot_t));

    if (!emitter->events)
    {
        EMITTER_FREE(emitter);
        return NULL;
    }

    memset(emitter->events, 0, (size_t)num_events * sizeof(emitter_slot_t));

    emitter->num_events = num_events;

    return emitter;
}

void emitter_destroy(emitter_t* emitter)
{
    EMITTER_ASSERT(emitter != NULL);

    for (int i = 0; i < emitter->num_events; i++)
    {
        EMITTER_FREE(emitter->events[i].listeners);
        EMITTER_FREE(emitter->events[i].udatas);
        EMITTER_FREE(emitter->events[i].once);
    }

    EMITTER_FREE(emitter->events);
    EMITTER_FREE(emitter);
}

void emitter_on(emitter_t* emitter, int event, emitter_listener_fn listener, void* udata)
{
    emitter_subscribe(emitter, event, listener, udata, 0);
}

void emitter_once(emitter_t* emitter, int event, emitter_listener_fn listener, void* udata)
{
    emitter_subscribe(emitter, event, listener, udata, 1);
}

void emitter_off(emitter_t* emitter, int event, emitter_listener_fn listener)
{
    EMITTER_ASSERT(emitter  != NULL);
    EMITTER_ASSERT(event    >= 0 && event < emitter->num_events);
    EMITTER_ASSERT(listener != NULL);

    emitter_slot_t* slot = &emitter->events[event];

    for (int i = 0; i < slot->count; i++)
    {
        if (slot->listeners[i] != listener)
        {
            continue;
        }

        if (slot->emitting)
        {
            // Tombstone; emitter_emit will compact after iteration.
            slot->listeners[i] = NULL;
        }
        else
        {
            // Compact in-place immediately.
            for (int j = i; j < slot->count - 1; j++)
            {
                slot->listeners[j] = slot->listeners[j + 1];
                slot->udatas[j]    = slot->udatas[j + 1];
                slot->once[j]      = slot->once[j + 1];
            }

            slot->count--;
        }

        return; // Remove first match only.
    }
}

void emitter_off_all(emitter_t* emitter, int event)
{
    EMITTER_ASSERT(emitter != NULL);
    EMITTER_ASSERT(event   >= 0 && event < emitter->num_events);

    emitter_slot_t* slot = &emitter->events[event];

    if (slot->emitting)
    {
        // Tombstone all; emitter_emit will compact after iteration.
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

void emitter_emit(emitter_t* emitter, int event, const void* data)
{
    EMITTER_ASSERT(emitter != NULL);
    EMITTER_ASSERT(event   >= 0 && event < emitter->num_events);

    emitter_slot_t* slot = &emitter->events[event];

    /*
     * Capture the count before iterating so that listeners added during emit
     * are not called in the current dispatch.
     */
    int  count         = slot->count;
    bool needs_compact = false;

    slot->emitting = true;

    for (int i = 0; i < count; i++)
    {
        emitter_listener_fn fn = slot->listeners[i];

        if (!fn)
        {
            // Tombstoned by emitter_off or emitter_off_all from a prior callback.
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
         * emitter_off during the callback. Check whether this slot became NULL
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
        emitter_compact(slot);
    }
}

int emitter_count(const emitter_t* emitter, int event)
{
    EMITTER_ASSERT(emitter != NULL);
    EMITTER_ASSERT(event   >= 0 && event < emitter->num_events);

    return emitter->events[event].count;
}

/* ==========================================================================
 * Queued emitter
 * ========================================================================== */

struct queued_emitter_s
{
    emitter_t*   emitter;
    arena_t      arena;
    int*         events;
    const void** datas;
    int          count;
    int          capacity;
};

static void queued_emitter_grow(queued_emitter_t* qe)
{
    int new_cap = qe->capacity == 0 ? EMITTER_INIT_CAPACITY : qe->capacity * 2;

    int*         new_events = (int*)arena_alloc(&qe->arena, (size_t)new_cap * sizeof(int));
    const void** new_datas  = (const void**)arena_alloc(&qe->arena, (size_t)new_cap * sizeof(const void*));

    EMITTER_ASSERT(new_events != NULL);
    EMITTER_ASSERT(new_datas  != NULL);

    if (qe->count > 0)
    {
        EMITTER_MEMCPY(new_events, qe->events, (size_t)qe->count * sizeof(int));
        EMITTER_MEMCPY(new_datas,  qe->datas,  (size_t)qe->count * sizeof(const void*));
    }

    qe->events   = new_events;
    qe->datas    = new_datas;
    qe->capacity = new_cap;
}

queued_emitter_t* queued_emitter_create(int num_events)
{
    EMITTER_ASSERT(num_events > 0);

    queued_emitter_t* qe = (queued_emitter_t*)EMITTER_MALLOC(sizeof(queued_emitter_t));

    if (!qe)
    {
        return NULL;
    }

    qe->emitter = emitter_create(num_events);

    if (!qe->emitter)
    {
        EMITTER_FREE(qe);
        return NULL;
    }

    size_t init_block = (size_t)EMITTER_INIT_CAPACITY * (sizeof(int) + sizeof(const void*));

    if (!arena_init(&qe->arena, init_block))
    {
        emitter_destroy(qe->emitter);
        EMITTER_FREE(qe);
        return NULL;
    }

    qe->events   = NULL;
    qe->datas    = NULL;
    qe->count    = 0;
    qe->capacity = 0;

    return qe;
}

void queued_emitter_destroy(queued_emitter_t* qe)
{
    EMITTER_ASSERT(qe != NULL);

    emitter_destroy(qe->emitter);
    arena_destroy(&qe->arena);
    EMITTER_FREE(qe);
}

void queued_emitter_on(queued_emitter_t* qe, int event, emitter_listener_fn listener, void* udata)
{
    EMITTER_ASSERT(qe != NULL);
    emitter_on(qe->emitter, event, listener, udata);
}

void queued_emitter_once(queued_emitter_t* qe, int event, emitter_listener_fn listener, void* udata)
{
    EMITTER_ASSERT(qe != NULL);
    emitter_once(qe->emitter, event, listener, udata);
}

void queued_emitter_off(queued_emitter_t* qe, int event, emitter_listener_fn listener)
{
    EMITTER_ASSERT(qe != NULL);
    emitter_off(qe->emitter, event, listener);
}

void queued_emitter_off_all(queued_emitter_t* qe, int event)
{
    EMITTER_ASSERT(qe != NULL);
    emitter_off_all(qe->emitter, event);
}

void queued_emitter_emit(queued_emitter_t* qe, int event, const void* data, size_t data_size)
{
    EMITTER_ASSERT(qe != NULL);
    EMITTER_ASSERT(event >= 0 && event < qe->emitter->num_events);

    if (qe->count == qe->capacity)
    {
        queued_emitter_grow(qe);
    }

    const void* stored = NULL;

    if (data != NULL && data_size > 0)
    {
        void* copy = arena_alloc(&qe->arena, data_size);
        EMITTER_ASSERT(copy != NULL);
        EMITTER_MEMCPY(copy, data, data_size);
        stored = copy;
    }

    qe->events[qe->count] = event;
    qe->datas[qe->count]  = stored;
    qe->count++;
}

void queued_emitter_flush(queued_emitter_t* qe)
{
    EMITTER_ASSERT(qe != NULL);

    // Capture count so that events enqueued during flush are deferred.
    int flush_count = qe->count;

    for (int i = 0; i < flush_count; i++)
    {
        emitter_emit(qe->emitter, qe->events[i], qe->datas[i]);
    }

    int remaining = qe->count - flush_count;

    if (remaining == 0)
    {
        // Common case: reset the arena, reclaiming all event-queue memory at once.
        arena_reset(&qe->arena);
        qe->events   = NULL;
        qe->datas    = NULL;
        qe->count    = 0;
        qe->capacity = 0;
    }
    else
    {
        // Events were enqueued during flush; copy them to fresh arena allocations
        // so the flushed region is logically released. The arena will be fully
        // reset on the next flush that completes with no remaining events.
        int*         src_events = qe->events + flush_count;
        const void** src_datas  = qe->datas  + flush_count;

        int*         new_events = (int*)arena_alloc(&qe->arena,
                                      (size_t)remaining * sizeof(int));

        const void** new_datas  = (const void**)arena_alloc(&qe->arena,
                                      (size_t)remaining * sizeof(const void*));

        EMITTER_ASSERT(new_events != NULL && new_datas != NULL);

        EMITTER_MEMCPY(new_events, src_events, (size_t)remaining * sizeof(int));
        EMITTER_MEMCPY(new_datas,  src_datas,  (size_t)remaining * sizeof(const void*));

        qe->events   = new_events;
        qe->datas    = new_datas;
        qe->count    = remaining;
        qe->capacity = remaining;
    }
}

int queued_emitter_count(const queued_emitter_t* qe, int event)
{
    EMITTER_ASSERT(qe != NULL);
    return emitter_count(qe->emitter, event);
}

/* ==========================================================================
 * Arena allocator
 * ========================================================================== */

static arena_block_t* arena_block_create(size_t size)
{
    arena_block_t* block = (arena_block_t*)EMITTER_MALLOC(sizeof(arena_block_t));

    if (!block)
        return NULL;

    block->memory = (uint8_t*)EMITTER_MALLOC(size);

    if (!block->memory)
    {
        EMITTER_FREE(block);
        return NULL;
    }

    block->size   = size;
    block->offset = 0;
    block->next   = NULL;

    return block;
}

static bool arena_init(arena_t* arena, size_t initial_block_size)
{
    arena_block_t* block = arena_block_create(initial_block_size);

    if (!block)
        return false;

    arena->first      = block;
    arena->current    = block;
    arena->block_size = initial_block_size;

    return true;
}

static bool arena_grow(arena_t* arena, size_t min_size)
{
    size_t new_size = arena->block_size;

    while (new_size < min_size)
    {
        new_size *= 2;
    }

    arena_block_t* block = arena_block_create(new_size);

    if (!block)
        return false;

    arena->current->next = block;
    arena->current       = block;

    return true;
}

static uintptr_t arena_align_forward(uintptr_t ptr, size_t align)
{
    uintptr_t mask = (uintptr_t)align - 1;
    return (ptr + mask) & ~mask;
}

static void* arena_alloc_align(arena_t* arena, size_t size, size_t align)
{
    if ((align & (align - 1)) != 0)
        return NULL; // align must be a power of two

    arena_block_t* block = arena->current;

    uintptr_t base      = (uintptr_t)block->memory;
    uintptr_t ptr       = base + block->offset;
    uintptr_t aligned   = arena_align_forward(ptr, align);
    size_t    new_offset = (aligned - base) + size;

    if (new_offset > block->size)
    {
        size_t required = size + align;

        if (!arena_grow(arena, required))
            return NULL;

        block      = arena->current;
        base       = (uintptr_t)block->memory;
        aligned    = arena_align_forward(base, align);
        new_offset = (aligned - base) + size;
    }

    block->offset = new_offset;

    return (void*)aligned;
}

static void* arena_alloc(arena_t* arena, size_t size)
{
    return arena_alloc_align(arena, size, alignof(max_align_t));
}

static void arena_reset(arena_t* arena)
{
    arena_block_t* block = arena->first->next;

    while (block)
    {
        arena_block_t* next = block->next;
        EMITTER_FREE(block->memory);
        EMITTER_FREE(block);
        block = next;
    }

    arena->first->next   = NULL;
    arena->first->offset = 0;
    arena->current       = arena->first;
}

static void arena_destroy(arena_t* arena)
{
    arena_block_t* block = arena->first;

    while (block)
    {
        arena_block_t* next = block->next;
        EMITTER_FREE(block->memory);
        EMITTER_FREE(block);
        block = next;
    }

    arena->first   = NULL;
    arena->current = NULL;
}

#endif // PICO_EMITTER_IMPLEMENTATION

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
