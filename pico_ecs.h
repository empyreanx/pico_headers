/**
    @file pico_ecs.h
    @brief A pure and simple ECS written in C99.

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Features:
    ---------
    - Written in C99
    - Single header library for easy build system integration
    - Excellent performance
    - Pure ECS design (strict separation between data and logic)
    - Simple and concise API
    - Permissive license (zlib or public domain)

    Summary:
    --------

    This library implements an ECS (Entity-Component-System). Entities
    (sometimes called game objects) are defined by their components. For
    example, an entity might have position, sprite, and physics components.
    Systems operate on the components of entities that match the system's
    requirements. Entities are matched to systems based upon which components
    they have and also the system's matching crieria.

    In the above example, a sprite renderer system would match entities having
    poition and sprite components. The system would send the appropriate
    geometry and texture ID to the game's graphics API.

    Traditional game engines tightly couple state with logic. For example, in
    C++ a game object typically has its own update method that operates
    on that state. They also usually rely heavily on inheritance.

    If the state specified by the class changes this could ripple through the
    class, and perhaps subclasses as well. It could well be that the class no
    longer belongs in the existing class hierarchy forcing even more revisions.
    If could even be true that a class doesn't neatly fit into the inheritance
    tree at all.

    An ECS solves these problems while also granting more flexibility in
    general.

    Please see the examples and unit tests for more details.

    Usage:
    ------

    To use this library in your project, add the following

    > #define PICO_ECS_IMPLEMENTATION
    > #include "pico_ecs.h"

    to a source file (once), then simply include the header normally.

    Constants:
    --------

    - PICO_ECS_MAX_COMPONENTS (default: 32)
    - PICO_ECS_MAX_ENTITIES (default: 8*1024)
    - PICO_ECS_MAX_SYSTEMS (default: 16)

    Must be defined before PICO_ECS_IMPLEMENTATION

    Todo:
    -----
    - Better default assertion macro
    - Port Rogue demo to Windows
*/

#ifndef PICO_ECS_H
#define PICO_ECS_H

#include <stdbool.h> // bool, true, false
#include <stdint.h>  // uint32_t

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ECS context
 */
typedef struct ecs_s ecs_t;

/**
 * @brief ID used for entity and components
 */
typedef uint32_t ecs_id_t;

/**
 * @brief Return code for update callback and calling functions
 */
typedef int8_t ecs_ret_t;

/**
 * @brief Invalid entity/component ID
 */
static const ecs_id_t ECS_NULL = (ecs_id_t)-1;

/**
 * @brief Determine floating point type
 */
#ifdef ECS_USE_FLOAT
    typedef float  ecs_dt_t;
#else
    typedef double ecs_dt_t;
#endif

/**
 * @brief System update callback
 *
 * Systems implement the core logic of an ECS by manipulating entities
 * and components.
 *
 * @param ecs          The ECS instance
 * @param entities     An array of entity IDs managed by the system
 * @param entity_count The number of entities in the array
 * @param dt           The time delta
 * @param udata        The user data associated with the system
 */
typedef ecs_ret_t (*ecs_update_fn)(ecs_t* ecs,
                                   ecs_id_t* entities,
                                   int entity_count,
                                   ecs_dt_t dt,
                                   void* udata);


/**
 * @brief Called when an entity is added to a system
 *
 * @param ecs       The ECS instance
 * @param entity_id The enitty being added
 * @param udata     The user data passed to the callback
 */
typedef void (*ecs_added_fn)(ecs_t* ecs, ecs_id_t entity_id, void * udata);

/**
 * @brief Called when an entity is removed from a system
 *
 * @param ecs       The ECS instance
 * @param entity_id The enitty being removed
 * @param udata     The user data passed to the callback
 */
typedef void (*ecs_removed_fn)(ecs_t* ecs, ecs_id_t entity_id, void *udata);

/**
 * @brief Creates an ECS instance.
 *
 * @returns An ECS instance or NULL if out of memory
 */
ecs_t* ecs_new(void* mem_ctx);

/**
 * @brief Destroys an ECS instance
 *
 * @param ecs The ECS instance
 */
void ecs_free(ecs_t* ecs);

/**
 * @brief Removes all entities from the ECS, preserving systems and components.
 */
void ecs_reset(ecs_t* ecs);

/**
 * @brief Registers a component
 *
 * Registers a component with the specfied component ID. Components define the
 * game state (usually contained within structs) and are manipulated by systems.
 *
 * @param ecs       The ECS instance
 * @param comp_id   The component ID to use (must be less than
 *                  ECS_MAX_COMPONENTS)
 * @param num_bytes The number of bytes to allocate for each component instance
 */
void ecs_register_component(ecs_t* ecs, ecs_id_t comp_id, int num_bytes);

/**
 * @brief Registers a system
 *
 * Registers a system with the user specified system ID. Systems contain the
 * core logic of a game by manipulating game state as defined by components.
 *
 * @param ecs       The ECS instance
 * @param sys_id    The system ID to use (must be less than ECS_MAX_SYSTEMS)
 * @param update_cb Callback that is fired every update
 * @param add_cb    Called when an entity is added to the system (can be NULL)
 * @param remove_cb Called when an entity is removed from the system (can be NULL)
 * @param udata     The user data passed to the callbacks
 */
void ecs_register_system(ecs_t* ecs,
                         ecs_id_t sys_id,
                         ecs_update_fn update_cb,
                         ecs_added_fn add_cb,
                         ecs_removed_fn remove_cb,
                         void* udata);
/**
 * @brief Determines which components are available to the specified system.
 *
 * @param ecs     The ECS instance
 * @param sys_id  The target system ID
 * @param comp_id The component ID
 */
void ecs_require_component(ecs_t* ecs, ecs_id_t sys_id, ecs_id_t comp_id);

/**
 * @brief Enables a system
 *
 * @param ecs    The ECS instance
 * @param sys_id The specified system ID
 */
void ecs_enable_system(ecs_t* ecs, ecs_id_t sys_id);

/**
 * @brief Disables a system
 *
 * @param ecs    The ECS instance
 * @param sys_id The specified system ID
 */
void ecs_disable_system(ecs_t* ecs, ecs_id_t sys_id);

/**
 * @brief Creates an entity
 *
 * @param ecs The ECS instance
 *
 * @returns The new entity ID or ECS_NULL if ECS_MAX_ENTITIES is reached
 */
ecs_id_t ecs_create(ecs_t* ecs);

/**
 * @brief Returns true if the entity is currently active
 *
 * @param ecs The ECS instance
 * @param entity_id The target entity
 */
bool ecs_is_ready(ecs_t* ecs, ecs_id_t entity_id);

/**
 * @brief Destroys an entity
 *
 * Destroys an entity, releasing resources and returning it to the pool.
 *
 * @param ecs       The ECS instance
 * @param entity_id The ID of the entity to destroy
 */
void ecs_destroy(ecs_t* ecs, ecs_id_t entity_id);

/**
 * @brief Queues an entity for destruction at the end of system execution
 *
 * Queued entities are destroyed after the curent iteration.
 *
 * @param ecs       The ECS instance
 * @param entity_id The ID of the entity to destroy
 */
void ecs_queue_destroy(ecs_t* ecs, ecs_id_t entity_id);

/**
 * @brief Test if entity has the specified component
 *
 * @param ecs       The ECS instance
 * @param entity_id The entity ID
 * @param comp_id   The component ID
 *
 * @returns True if the entity has the component
 */
bool ecs_has(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id);

/**
 * @brief Adds a component instance to an entity
 *
 * @param ecs       The ECS instance
 * @param entity_id The entity ID
 * @param comp_id   The component ID
 *
 * @returns The component instance
 */
void* ecs_add(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id);

/**
 * @brief Gets a component instance associated with an entity
 *
 * @param ecs       The ECS instance
 * @param entity_id The entity ID
 * @param comp_id   The component ID
 *
 * @returns The component instance
 */
void* ecs_get(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id);

/**
 * @brief Removes a component instance from an entity
 *
 * @param ecs       The ECS instance
 * @param entity_id The entity ID
 * @param comp_id   The component ID
 */
void ecs_remove(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id);

/**
 * @brief Adds/removes an entity from systems based upon its components
 *
 * @param ecs The ECS instance
 * @param entity_id The target entity
 */
void ecs_sync(ecs_t* ecs, ecs_id_t entity_id);

/**
 * @brief Queues a sync call on the specified entity at the end of system
 * execution
 *
 * Queued entities are synced after the curent iteration.
 *
 * @param ecs       The ECS instance
 * @param entity_id The ID of the entity to sync
 */
void ecs_queue_sync(ecs_t* ecs, ecs_id_t entity_id);

/**
 * @brief Update an individual system
 *
 * This function should be called once per frame.
 *
 * @param ecs The ECS instance
 * @param sys_id The system to update
 * @param dt  The time delta
 */
ecs_ret_t ecs_update_system(ecs_t* ecs, ecs_id_t sys_id, ecs_dt_t dt);

/**
 * @brief Updates all systems
 *
 * This function should be called once per frame.
 *
 * @param ecs The ECS instance
 * @param dt  The time delta
 */
ecs_ret_t ecs_update_systems(ecs_t* ecs, ecs_dt_t dt);

#ifdef __cplusplus
}
#endif

#endif // PICO_ECS_H

#ifdef PICO_ECS_IMPLEMENTATION // Define once

#include <stdint.h> // uint32_t
#include <stdlib.h> // malloc, realloc, free
#include <string.h> // memcpy, memset

#ifndef PICO_ECS_MAX_COMPONENTS
#define PICO_ECS_MAX_COMPONENTS 32
#endif

#ifndef PICO_ECS_MAX_ENTITIES
#define PICO_ECS_MAX_ENTITIES (8*1024)
#endif

#ifndef PICO_ECS_MAX_SYSTEMS
#define PICO_ECS_MAX_SYSTEMS 16
#endif

#ifdef NDEBUG
    #define PICO_ECS_ASSERT(expr) ((void)0)
#else
    #ifndef PICO_ECS_ASSERT
        #include <assert.h>
        #define PICO_ECS_ASSERT(expr) (assert(expr))
    #endif
#endif

#if !defined(PICO_ECS_MALLOC) || !defined(PICO_ECS_FREE)
#include <stdlib.h>
#define PICO_ECS_MALLOC(size, ctx) (malloc(size))
#define PICO_ECS_FREE(ptr, ctx)    (free(ptr))
#endif

/*=============================================================================
 * Internal aliases
 *============================================================================*/

#define ECS_ASSERT          PICO_ECS_ASSERT
#define ECS_MAX_COMPONENTS  PICO_ECS_MAX_COMPONENTS
#define ECS_MAX_ENTITIES    PICO_ECS_MAX_ENTITIES
#define ECS_MAX_SYSTEMS     PICO_ECS_MAX_SYSTEMS
#define ECS_ASSERT          PICO_ECS_ASSERT
#define ECS_MALLOC          PICO_ECS_MALLOC
#define ECS_FREE            PICO_ECS_FREE

/*=============================================================================
 * Internal data structures
 *============================================================================*/

#if ECS_MAX_COMPONENTS <= 32
typedef uint32_t ecs_bitset_t;
#elif ECS_MAX_COMPONENTS <= 64
typedef uint64_t ecs_bitset_t;
#else
#define ECS_BITSET_WIDTH 64
#define ECS_BITSET_SIZE (((ECS_MAX_COMPONENTS - 1) / ECS_BITSET_WIDTH) + 1)

typedef struct
{
    uint64_t array[ECS_BITSET_SIZE];
} ecs_bitset_t;
#endif // ECS_MAX_COMPONENTS

// Data-structure for a packed array implementation that provides O(1) functions
// for adding, removing, and accessing entity IDs
typedef struct
{
    ecs_id_t size;
    ecs_id_t sparse[ECS_MAX_ENTITIES];
    ecs_id_t dense[ECS_MAX_ENTITIES];
} ecs_sparse_set_t;

// Data-structure for and ID pool that provides O(1) operations for
// pooling/queueing IDs
typedef struct
{
    int   size;
    ecs_id_t array[ECS_MAX_ENTITIES];
} ecs_id_stack_t;

typedef struct
{
    ecs_bitset_t comp_bits;
    bool         ready;
} ecs_entity_t;

typedef struct
{
    void* array;
    int   size;
    bool  ready;
} ecs_comp_t;

typedef struct
{
    bool             ready;
    bool             active;
    ecs_sparse_set_t entity_ids;
    ecs_update_fn    update_cb;
    ecs_added_fn     add_cb;
    ecs_removed_fn   remove_cb;
    ecs_bitset_t     comp_bits;
    void*            udata;
} ecs_sys_t;

struct ecs_s
{
    ecs_id_stack_t entity_pool;
    ecs_id_stack_t destroy_queue;
    ecs_id_stack_t sync_queue;
    ecs_entity_t   entities[ECS_MAX_ENTITIES];
    ecs_comp_t     comps[ECS_MAX_COMPONENTS];
    ecs_sys_t      systems[ECS_MAX_SYSTEMS];
    void*          mem_ctx;
};

/*=============================================================================
 * Internal functions to flush destroyed entity and removed component
 *============================================================================*/
static void ecs_flush_sync(ecs_t* ecs);
static void ecs_flush_destroyed(ecs_t* ecs);

/*=============================================================================
 * Internal bit set functions
 *============================================================================*/
static inline void  ecs_bitset_flip(ecs_bitset_t* set, int bit, bool on);
static inline bool  ecs_bitset_test(ecs_bitset_t* set, int bit);
static inline ecs_bitset_t ecs_bitset_and(ecs_bitset_t* set1, ecs_bitset_t* set2);
static inline bool  ecs_bitset_equal(ecs_bitset_t* set1, ecs_bitset_t* set2);
static inline bool  ecs_bitset_true(ecs_bitset_t* set);

/*=============================================================================
 * Internal sparse set functions
 *============================================================================*/
static void     ecs_sparse_set_init(ecs_sparse_set_t* set);
static ecs_id_t ecs_sparse_set_find(ecs_sparse_set_t* set, ecs_id_t id);
static bool     ecs_sparse_set_add(ecs_sparse_set_t* set, ecs_id_t id);
static bool     ecs_sparse_set_remove(ecs_sparse_set_t* set, ecs_id_t id);

/*=============================================================================
 * Internal system entity add/remove functions
 *============================================================================*/
static bool ecs_entity_system_test(ecs_bitset_t* sys_bits,
                                   ecs_bitset_t* entity_bits);

static void ecs_remove_entity_from_systems(ecs_t* ecs, ecs_id_t entity_id);

/*=============================================================================
 * Internal ID pool functions
 *============================================================================*/
static void     ecs_id_stack_push(ecs_id_stack_t* pool, ecs_id_t id);
static ecs_id_t ecs_id_stack_pop(ecs_id_stack_t* pool);
static int      ecs_id_stack_size(ecs_id_stack_t* pool);

/*=============================================================================
 * Internal validation functions
 *============================================================================*/
#ifndef NDEBUG
static bool ecs_is_not_null(void* ptr);
static bool ecs_is_valid_entity_id(ecs_id_t id);
static bool ecs_is_valid_component_id(ecs_id_t id);
static bool ecs_is_valid_system_id(ecs_id_t id);
static bool ecs_is_entity_ready(ecs_t* ecs, ecs_id_t entity_id);
static bool ecs_is_component_ready(ecs_t* ecs, ecs_id_t comp_id);
static bool ecs_is_system_ready(ecs_t* ecs, ecs_id_t sys_id);
#endif // NDEBUG
/*=============================================================================
 * Public API implementation
 *============================================================================*/

ecs_t* ecs_new(void* mem_ctx)
{
    ecs_t* ecs = (ecs_t*)ECS_MALLOC(sizeof(ecs_t), mem_ctx);

    // Out of memory
    if (NULL == ecs)
        return NULL;

    memset(ecs, 0, sizeof(ecs_t));

    ecs->mem_ctx = mem_ctx;

    // Pre-populate the the ID pools
    for (ecs_id_t entity_id = 0; entity_id < ECS_MAX_ENTITIES; entity_id++)
    {
        ecs_id_stack_push(&ecs->entity_pool, entity_id);
    }

    return ecs;
}

void ecs_free(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    for (ecs_id_t comp_id = 0; comp_id < ECS_MAX_COMPONENTS; comp_id++)
    {
        ecs_comp_t* comp = &ecs->comps[comp_id];

        if (NULL != comp->array)
        {
            ECS_FREE(ecs->comps[comp_id].array, ecs->mem_ctx);
        }
    }

    ECS_FREE(ecs, ecs->mem_ctx);
}

void ecs_reset(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    ecs->entity_pool.size = 0;
    ecs->destroy_queue.size = 0;
    ecs->sync_queue.size = 0;

    memset(ecs->entities, 0, sizeof(ecs_entity_t) * ECS_MAX_ENTITIES);

    for (ecs_id_t entity_id = 0; entity_id < ECS_MAX_ENTITIES; entity_id++)
    {
        ecs_id_stack_push(&ecs->entity_pool, entity_id);
    }

    for (ecs_id_t sys_id = 0; sys_id < ECS_MAX_SYSTEMS; sys_id++)
    {
        ecs_sparse_set_init(&ecs->systems[sys_id].entity_ids);
    }
}

void ecs_register_component(ecs_t* ecs, ecs_id_t comp_id, int num_bytes)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_component_id(comp_id));
    ECS_ASSERT(!ecs_is_component_ready(ecs, comp_id));

    ecs_comp_t* comp = &ecs->comps[comp_id];

    comp->array = ECS_MALLOC(ECS_MAX_ENTITIES * num_bytes, ecs->mem_ctx);
    comp->size  = num_bytes;
    comp->ready = true;

    memset(comp->array, 0, ECS_MAX_ENTITIES * num_bytes);
}

void ecs_register_system(ecs_t* ecs,
                         ecs_id_t sys_id,
                         ecs_update_fn update_cb,
                         ecs_added_fn add_cb,
                         ecs_removed_fn remove_cb,

                         void* udata)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys_id));
    ECS_ASSERT(!ecs_is_system_ready(ecs, sys_id));
    ECS_ASSERT(NULL != update_cb);

    ecs_sys_t* sys = &ecs->systems[sys_id];

    ecs_sparse_set_init(&sys->entity_ids);

    sys->ready = true;
    sys->active = true;
    sys->update_cb = update_cb;
    sys->add_cb = add_cb;
    sys->remove_cb = remove_cb;
    sys->udata = udata;
}

void ecs_require_component(ecs_t* ecs, ecs_id_t sys_id, ecs_id_t comp_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys_id));
    ECS_ASSERT(ecs_is_valid_component_id(comp_id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys_id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp_id));

    // Load system
    ecs_sys_t* sys = &ecs->systems[sys_id];

    // Set system component bit for the specified component
    ecs_bitset_flip(&sys->comp_bits, comp_id, true);
}

void ecs_enable_system(ecs_t* ecs, ecs_id_t sys_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys_id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys_id));

    ecs_sys_t* sys = &ecs->systems[sys_id];
    sys->active = true;
}

void ecs_disable_system(ecs_t* ecs, ecs_id_t sys_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys_id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys_id));

    ecs_sys_t* sys = &ecs->systems[sys_id];
    sys->active = false;
}

ecs_id_t ecs_create(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    ecs_id_stack_t* pool = &ecs->entity_pool;

    ECS_ASSERT(ecs_id_stack_size(pool) > 0);

    if (0 == ecs_id_stack_size(pool))
        return ECS_NULL;

    ecs_id_t entity_id = ecs_id_stack_pop(pool);
    ecs->entities[entity_id].ready = true;

    return entity_id;
}

bool ecs_is_ready(ecs_t* ecs, ecs_id_t entity_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_entity_id(entity_id));

    return ecs->entities[entity_id].ready;
}

void ecs_destroy(ecs_t* ecs, ecs_id_t entity_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_entity_id(entity_id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    // Load entity
    ecs_entity_t* entity = &ecs->entities[entity_id];

    // Remove entity from systems
    ecs_remove_entity_from_systems(ecs, entity_id);

    // Push entity ID back into pool
    ecs_id_stack_t* pool = &ecs->entity_pool;
    ecs_id_stack_push(pool, entity_id);

    // Reset entity
    memset(entity, 0, sizeof(ecs_entity_t));
}

void ecs_queue_destroy(ecs_t* ecs, ecs_id_t entity_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_entity_id(entity_id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    // Push entity ID onto destroy_queue
    ecs_id_stack_push(&ecs->destroy_queue, entity_id);

    ecs->entities[entity_id].ready = false;
}

bool ecs_has(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_entity_id(entity_id));
    ECS_ASSERT(ecs_is_valid_component_id(comp_id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    // Load  entity
    ecs_entity_t* entity = &ecs->entities[entity_id];

    // Return true if the component belongs to the entity
    return ecs_bitset_test(&entity->comp_bits, comp_id);
}

void* ecs_get(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_entity_id(entity_id));
    ECS_ASSERT(ecs_is_valid_component_id(comp_id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp_id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    // Return pointer to component
    ecs_comp_t* comp = &ecs->comps[comp_id]; //  eid0,  eid1   eid2, ...
                                             // [comp0, comp1, comp2, ...]
    return (char*)comp->array + (comp->size * entity_id);
}

void* ecs_add(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_entity_id(entity_id));
    ECS_ASSERT(ecs_is_valid_component_id(comp_id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp_id));

    // Load entity
    ecs_entity_t* entity = &ecs->entities[entity_id];
    ecs_comp_t* comp = &ecs->comps[comp_id];

    // Set entity component bit that determines which systems this entity
    // belongs to
    ecs_bitset_flip(&entity->comp_bits, comp_id, true);

    // Add entity to systems
    for (ecs_id_t sys_id = 0; sys_id < ECS_MAX_SYSTEMS; sys_id++)
    {
        ecs_sys_t* sys = &ecs->systems[sys_id];

        if (!sys->ready)
            continue;

        if (ecs_bitset_test(&sys->comp_bits, comp_id))
        {
            if (ecs_sparse_set_add(&sys->entity_ids, entity_id))
            {
                if (sys->add_cb)
                    sys->add_cb(ecs, entity_id, sys->udata);
            }
        }
    }

    // Allocate component

    // Get pointer to component
    void* ptr = ecs_get(ecs, entity_id, comp_id);

    // Reset component
    memset(ptr, 0, comp->size);

    // Return component
    return ptr;
}

void ecs_remove(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_entity_id(entity_id));
    ECS_ASSERT(ecs_is_valid_component_id(comp_id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp_id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    // Load entity
    ecs_entity_t* entity = &ecs->entities[entity_id];

    // Remove entity from systems
    for (ecs_id_t sys_id = 0; sys_id < ECS_MAX_SYSTEMS; sys_id++)
    {
        ecs_sys_t* sys = &ecs->systems[sys_id];

        if (!sys->ready)
            continue;

        if (ecs_bitset_test(&sys->comp_bits, comp_id))
        {
            if (ecs_sparse_set_remove(&sys->entity_ids, entity_id))
            {
                if (sys->add_cb)
                    sys->add_cb(ecs, entity_id, sys->udata);
            }
        }
    }

    // Reset the relevant component mask bit
    ecs_bitset_flip(&entity->comp_bits, comp_id, false);

    // Deallocate component
}

void ecs_sync(ecs_t* ecs, ecs_id_t entity_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_entity_id(entity_id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    ecs_bitset_t* entity_bits = &ecs->entities[entity_id].comp_bits;

    for (ecs_id_t sys_id = 0; sys_id < ECS_MAX_SYSTEMS; sys_id++)
    {
        if (!ecs->systems[sys_id].ready)
            continue;

        ecs_sys_t* sys = &ecs->systems[sys_id];

        if (ecs_entity_system_test(&sys->comp_bits, entity_bits))
        {
            if (ecs_sparse_set_add(&sys->entity_ids, entity_id))
            {
                if (sys->add_cb)
                    sys->add_cb(ecs, entity_id, sys->udata);
            }
        }
        else
        {
            if (ecs_sparse_set_remove(&sys->entity_ids, entity_id))
            {
                if (sys->remove_cb)
                    sys->remove_cb(ecs, entity_id, sys->udata);
            }
        }
    }
}

void ecs_queue_sync(ecs_t* ecs, ecs_id_t entity_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_entity_id(entity_id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    // Push both component and entities onto ID stacks
    ecs_id_stack_push(&ecs->sync_queue, entity_id);
}

ecs_ret_t ecs_update_system(ecs_t* ecs, ecs_id_t sys_id, ecs_dt_t dt)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys_id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys_id));
    ECS_ASSERT(dt >= 0.0f);

    ecs_sys_t* sys = &ecs->systems[sys_id];

    if (!sys->active)
        return 0;

    ecs_ret_t code = sys->update_cb(ecs,
                     sys->entity_ids.dense,
                     sys->entity_ids.size,
                     dt,
                     sys->udata);

    ecs_flush_sync(ecs);
    ecs_flush_destroyed(ecs);

    return code;
}

ecs_ret_t ecs_update_systems(ecs_t* ecs, ecs_dt_t dt)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(dt >= 0.0f);

    for (ecs_id_t sys_id = 0; sys_id < ECS_MAX_SYSTEMS; sys_id++)
    {
        ecs_sys_t* sys = &ecs->systems[sys_id];

        if (!sys->ready)
            continue;

        ecs_ret_t code = ecs_update_system(ecs, sys_id, dt);

        if (0 != code)
            return code;
    }

    return 0;
}

/*=============================================================================
 * Internal functions to flush destroyed entity and removed component
 *============================================================================*/

static void ecs_flush_sync(ecs_t* ecs)
{
    ecs_id_stack_t* entities = &ecs->sync_queue;

    while (ecs_id_stack_size(entities) > 0)
    {
        ecs_id_t entity_id = ecs_id_stack_pop(entities);
        ecs_sync(ecs, entity_id);
    }
}

static void ecs_flush_destroyed(ecs_t* ecs)
{
    ecs_id_stack_t* entities = &ecs->destroy_queue;

    while (ecs_id_stack_size(entities) > 0)
    {
        ecs_id_t entity_id = ecs_id_stack_pop(entities);
        ecs->entities[entity_id].ready = true; // FIXME: this is a hack
        ecs_destroy(ecs, entity_id);
    }
}

/*=============================================================================
 * Internal bitset functions
 *============================================================================*/

#if ECS_MAX_COMPONENTS <= 64

static inline void ecs_bitset_flip(ecs_bitset_t* set, int bit, bool on)
{
    if (on)
        *set |=  (1 << bit);
    else
        *set &= ~(1 << bit);
}

static inline bool ecs_bitset_test(ecs_bitset_t* set, int bit)
{
    return *set & (1 << bit);
}

static inline ecs_bitset_t ecs_bitset_and(ecs_bitset_t* set1, ecs_bitset_t* set2)
{
    return *set1 & *set2;
}

static inline bool ecs_bitset_equal(ecs_bitset_t* set1, ecs_bitset_t* set2)
{
    return *set1 == *set2;
}

static inline bool ecs_bitset_true(ecs_bitset_t* set)
{
    return *set;
}

#else // ECS_MAX_COMPONENTS

static inline void ecs_bitset_flip(ecs_bitset_t* set, int bit, bool on)
{
    int index = bit / ECS_BITSET_WIDTH;

    if (on)
        set->array[index] |=  (1 << bit % ECS_BITSET_WIDTH);
    else
        set->array[index] &= ~(1 << bit % ECS_BITSET_WIDTH);
}

static inline bool ecs_bitset_test(ecs_bitset_t* set, int bit)
{
    int index = bit / ECS_BITSET_WIDTH;
    return set->array[index] & (1 << bit % ECS_BITSET_WIDTH);
}

static inline ecs_bitset_t ecs_bitset_and(ecs_bitset_t* set1, ecs_bitset_t* set2)
{
    ecs_bitset_t set;

    for (int i = 0; i < ECS_BITSET_SIZE; i++)
    {
        set.array[i] = set1->array[i] & set2->array[i];
    }

    return set;
}

static inline bool ecs_bitset_equal(ecs_bitset_t* set1, ecs_bitset_t* set2)
{
    for (int i = 0; i < ECS_BITSET_SIZE; i++)
    {
        if (set1->array[i] != set2->array[i])
        {
            return false;
        }
    }

    return true;
}

static inline bool ecs_bitset_true(ecs_bitset_t* set)
{
    for (int i = 0; i < ECS_BITSET_SIZE; i++)
    {
        if (set->array[i])
            return true;
    }

    return false;
}

#endif // ECS_MAX_COMPONENTS

/*=============================================================================
 * Internal sparse set functions
 *============================================================================*/

static void ecs_sparse_set_init(ecs_sparse_set_t* set)
{
    ECS_ASSERT(ecs_is_not_null(set));

    memset(set, 0, sizeof(ecs_sparse_set_t));

    for (ecs_id_t id = 0; id < ECS_MAX_ENTITIES; id++)
    {
        set->sparse[id] = ECS_NULL;
    }
}

static ecs_id_t ecs_sparse_set_find(ecs_sparse_set_t* set, ecs_id_t id)
{
    ECS_ASSERT(ecs_is_not_null(set));

    if (set->sparse[id] < set->size && set->dense[set->sparse[id]] == id)
        return set->sparse[id];
    else
        return ECS_NULL;
}

static bool ecs_sparse_set_add(ecs_sparse_set_t* set, ecs_id_t id)
{
    ECS_ASSERT(ecs_is_not_null(set));

    if (ECS_NULL != ecs_sparse_set_find(set, id))
        return false;

    set->dense[set->size] = id;
    set->sparse[id] = set->size;

    set->size++;

    return true;
}

static bool ecs_sparse_set_remove(ecs_sparse_set_t* set, ecs_id_t id)
{
    ECS_ASSERT(ecs_is_not_null(set));

    if (ECS_NULL == ecs_sparse_set_find(set, id))
        return false;

    // Swap and remove (changes order of array)
    ecs_id_t tmp = set->dense[set->size - 1];
    set->dense[set->sparse[id]] = tmp;
    set->sparse[tmp] = set->sparse[id];

    set->size--;

    return true;
}

/*=============================================================================
 * Internal system entity add/remove functions
 *============================================================================*/

inline static bool ecs_entity_system_test(ecs_bitset_t* sys_bits,
                                          ecs_bitset_t* entity_bits)
{
    ecs_bitset_t tmp = ecs_bitset_and(sys_bits, entity_bits);
    return ecs_bitset_equal(sys_bits, &tmp);
}

static void ecs_remove_entity_from_systems(ecs_t* ecs, ecs_id_t entity_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    for (ecs_id_t sys_id = 0; sys_id < ECS_MAX_SYSTEMS; sys_id++)
    {
        if (!ecs->systems[sys_id].ready)
            continue;

        ecs_sys_t* sys = &ecs->systems[sys_id];

        // Just attempting to remove the entity from the sparse set is faster
        // than calling ecs_entity_system_test
        if (ecs_sparse_set_remove(&sys->entity_ids, entity_id))
        {
            if (sys->remove_cb)
                sys->remove_cb(ecs, entity_id, sys->udata);
        }
    }
}

/*=============================================================================
 * Internal ID pool functions
 *============================================================================*/

inline static void ecs_id_stack_push(ecs_id_stack_t* stack, ecs_id_t id)
{
    ECS_ASSERT(ecs_is_not_null(stack));
    ECS_ASSERT(stack->size < ECS_MAX_ENTITIES);
    stack->array[stack->size++] = id;
}

inline static ecs_id_t ecs_id_stack_pop(ecs_id_stack_t* stack)
{
    ECS_ASSERT(ecs_is_not_null(stack));
    return stack->array[--stack->size];
}

inline static int ecs_id_stack_size(ecs_id_stack_t* stack)
{
    return stack->size;
}

/*=============================================================================
 * Internal validation functions
 *============================================================================*/
#ifndef NDEBUG
static bool ecs_is_not_null(void* ptr)
{
    return NULL != ptr;
}

static bool ecs_is_valid_entity_id(ecs_id_t id)
{
    return id < ECS_MAX_ENTITIES;
}

static bool ecs_is_valid_component_id(ecs_id_t id)
{
    return id < ECS_MAX_COMPONENTS;
}

static bool ecs_is_valid_system_id(ecs_id_t id)
{
    return id < ECS_MAX_SYSTEMS;
}

static bool ecs_is_entity_ready(ecs_t* ecs, ecs_id_t entity_id)
{
    return ecs->entities[entity_id].ready;
}

static bool ecs_is_component_ready(ecs_t* ecs, ecs_id_t comp_id)
{
    return ecs->comps[comp_id].ready;
}

static bool ecs_is_system_ready(ecs_t* ecs, ecs_id_t sys_id)
{
    return ecs->systems[sys_id].ready;
}

#endif // NDEBUG

#endif // PICO_ECS_IMPLEMENTATION

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

// EoF
