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

    > #define ECS_IMPLEMENTATION
    > #include "pico_ecs.h"

    to a source file (once), then simply include the header normally.

    Constants:
    --------

    - ECS_MAX_COMPONENTS (default: 16)
    - ECS_MAX_ENTITIES (default: 8*1024)
    - ECS_MAX_SYSTEMS (default: 16)

    Must be defined before ECS_IMPLEMENTATION

    Todo:
    -----
    - Better default assertion macro
    - Use ECS_MATCH_ANY for player system. Logic to collect chests.
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
 * @brief System matching criteria
 *
 * Determines which entities are added to a system according to what components
 * an entity has, and which components a system requires.
 */
typedef enum
{
    ECS_MATCH_NONE,     /*!< No entities match */
    ECS_MATCH_ALL,      /*!< All entities match */
    ECS_MATCH_ANY,      /*!< Matches entities having any of the components
                             assigned to the system */
    ECS_MATCH_REQUIRED, /*!< Matches entities that have a superset of the
                             components assigned to the system */
    ECS_MATCH_EXACT     /*!< Matches entities must have extactly the same
                             components as those assigned to the system */
} ecs_match_t;

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
 * @param udata     The user data passed to the update callback
 */
void ecs_register_system(ecs_t* ecs,
                         ecs_id_t sys_id,
                         ecs_update_fn update_cb,
                         ecs_match_t match,
                         void* udata);
/**
 * @brief Determines which components are available to the specified system.
 *
 * @param ecs     The ECS instance
 * @param sys_id  The target system ID
 * @param comp_id The component ID
 */
void ecs_match_component(ecs_t* ecs, ecs_id_t sys_id, ecs_id_t comp_id);

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

#ifdef ECS_IMPLEMENTATION // Define once

#include <stdint.h> // uint32_t
#include <stdlib.h> // malloc, realloc, free
#include <string.h> // memcpy, memset

#ifndef ECS_MAX_COMPONENTS
#define ECS_MAX_COMPONENTS 16
#endif

#ifndef ECS_MAX_ENTITIES
#define ECS_MAX_ENTITIES (8*1024)
#endif

#ifndef ECS_MAX_SYSTEMS
#define ECS_MAX_SYSTEMS 16
#endif

#ifdef ECS_DEBUG
    #ifndef ECS_ASSERT
        #include <assert.h>
        #define ECS_ASSERT(expr) (assert(expr))
    #endif
#else
    #define ECS_ASSERT(expr) ((void)0)
#endif

#if !defined(ECS_MALLOC) || !defined(ECS_FREE)
#include <stdlib.h>
#define ECS_MALLOC(size, ctx) (malloc(size))
#define ECS_FREE(ptr, ctx)    (free(ptr))
#endif

/*=============================================================================
 * Internal data structures
 *============================================================================*/

#if ECS_MAX_COMPONENTS <= 32
   typedef uint32_t ecs_bitset_t;
#elif ECS_MAX_COMPONENTS <= 64
   typedef uint64_t ecs_bitset_t;
#else
    #error "Too many components"
#endif

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
    void*  array;
    int size;
    bool   ready;
} ecs_comp_t;

typedef struct
{
    bool             ready;
    bool             active;
    ecs_sparse_set_t entity_ids;
    ecs_update_fn    update_cb;
    ecs_match_t      match;
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
static ecs_bitset_t ecs_bitset_flip(ecs_bitset_t set, int bit, bool on);
static bool         ecs_bitset_test(ecs_bitset_t set, int bit);

/*=============================================================================
 * Internal sparse set functions
 *============================================================================*/
static void     ecs_sparse_set_init(ecs_sparse_set_t* set);
static ecs_id_t ecs_sparse_set_find(ecs_sparse_set_t* set, ecs_id_t id);
static void     ecs_sparse_set_add(ecs_sparse_set_t* set, ecs_id_t id);
static void     ecs_sparse_set_remove(ecs_sparse_set_t* set, ecs_id_t id);

/*=============================================================================
 * Internal system entity add/remove functions
 *============================================================================*/
static bool ecs_entity_system_test(ecs_match_t match,
                                   ecs_bitset_t sys_bits,
                                   ecs_bitset_t entity_bits);

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
#ifdef ECS_DEBUG
static bool ecs_is_not_null(void* ptr);
static bool ecs_is_valid_entity_id(ecs_id_t id);
static bool ecs_is_valid_component_id(ecs_id_t id);
static bool ecs_is_valid_system_id(ecs_id_t id);
static bool ecs_is_entity_ready(ecs_t* ecs, ecs_id_t entity_id);
static bool ecs_is_component_ready(ecs_t* ecs, ecs_id_t comp_id);
static bool ecs_is_system_ready(ecs_t* ecs, ecs_id_t sys_id);
#endif // ECS_DEBUG
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
    ECS_ASSERT(is_not_null(ecs));

    for (ecs_id_t comp_id = 0; comp_id < ECS_MAX_COMPONENTS; comp_id++)
    {
        if (NULL != ecs->comps[comp_id].array)
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
}

void ecs_register_system(ecs_t* ecs,
                         ecs_id_t sys_id,
                         ecs_update_fn update_cb,
                         ecs_match_t match,
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
    sys->match = match;
    sys->udata = udata;
}

void ecs_match_component(ecs_t* ecs, ecs_id_t sys_id, ecs_id_t comp_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys_id));
    ECS_ASSERT(ecs_is_valid_component_id(comp_id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys_id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp_id));

    // Load system
    ecs_sys_t* sys = &ecs->systems[sys_id];

    // Set system component bit for the specified component
    sys->comp_bits = ecs_bitset_flip(sys->comp_bits, comp_id, true);
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
    ECS_ASSERT(is_not_null(ecs));

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
    return ecs_bitset_test(entity->comp_bits, comp_id);
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

    // Set entity component bit that determines which systems this entity
    // belongs to
    entity->comp_bits = ecs_bitset_flip(entity->comp_bits, comp_id, true);

    // Return pointer to component
    return ecs_get(ecs, entity_id, comp_id);
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

    // Reset the relevant component mask bit
    entity->comp_bits = ecs_bitset_flip(entity->comp_bits, comp_id, false);
}

void ecs_sync(ecs_t* ecs, ecs_id_t entity_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_entity_id(entity_id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    ecs_bitset_t entity_bits = ecs->entities[entity_id].comp_bits;

    for (ecs_id_t sys_id = 0; sys_id < ECS_MAX_SYSTEMS; sys_id++)
    {
        if (!ecs->systems[sys_id].ready)
            continue;

        ecs_sys_t* sys = &ecs->systems[sys_id];

        if (ecs_entity_system_test(sys->match, sys->comp_bits, entity_bits))
            ecs_sparse_set_add(&sys->entity_ids, entity_id);
        else
            ecs_sparse_set_remove(&sys->entity_ids, entity_id);
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

static ecs_bitset_t ecs_bitset_flip(ecs_bitset_t set, int bit, bool on)
{
    if (on)
        set |=  (1 << bit);
    else
        set &= ~(1 << bit);

    return set;
}

static bool ecs_bitset_test(ecs_bitset_t set, int bit)
{
    return set & (1 << bit);
}

/*=============================================================================
 * Internal sparse set functions
 *============================================================================*/

static void ecs_sparse_set_init(ecs_sparse_set_t* set)
{
    ECS_ASSERT(is_not_null(set));

    memset(set, 0, sizeof(ecs_sparse_set_t));

    for (ecs_id_t id = 0; id < ECS_MAX_ENTITIES; id++)
    {
        set->sparse[id] = ECS_NULL;
    }
}

static ecs_id_t ecs_sparse_set_find(ecs_sparse_set_t* set, ecs_id_t id)
{
    ECS_ASSERT(is_not_null(set));

    if (set->sparse[id] < set->size && set->dense[set->sparse[id]] == id)
        return set->sparse[id];
    else
        return ECS_NULL;
}

static void ecs_sparse_set_add(ecs_sparse_set_t* set, ecs_id_t id)
{
    ECS_ASSERT(is_not_null(set));

    if (ECS_NULL != ecs_sparse_set_find(set, id))
        return;

    set->dense[set->size] = id;
    set->sparse[id] = set->size;

    set->size++;
}

static void ecs_sparse_set_remove(ecs_sparse_set_t* set, ecs_id_t id)
{
    ECS_ASSERT(is_not_null(set));

    if (ECS_NULL == ecs_sparse_set_find(set, id))
        return;

    // Swap and remove (changes order of array)
    ecs_id_t tmp = set->dense[set->size - 1];
    set->dense[set->sparse[id]] = tmp;
    set->sparse[tmp] = set->sparse[id];

    set->size--;
}

/*=============================================================================
 * Internal system entity add/remove functions
 *============================================================================*/

inline static bool ecs_entity_system_test(ecs_match_t match,
                                          ecs_bitset_t sys_bits,
                                          ecs_bitset_t entity_bits)
{
    switch (match)
    {
        case ECS_MATCH_NONE:
            return false;

        case ECS_MATCH_ALL:
            return true;

        case ECS_MATCH_ANY:
            return sys_bits & entity_bits;

        case ECS_MATCH_REQUIRED:
            return sys_bits == (sys_bits & entity_bits);

        case ECS_MATCH_EXACT:
            return sys_bits == entity_bits;

        default:
            ECS_ASSERT(false);
            return false;
    }
}

static void ecs_remove_entity_from_systems(ecs_t* ecs, ecs_id_t entity_id)
{
    ECS_ASSERT(is_not_null(ecs));

    for (ecs_id_t sys_id = 0; sys_id < ECS_MAX_SYSTEMS; sys_id++)
    {
        if (!ecs->systems[sys_id].ready)
            continue;

        ecs_sys_t* sys = &ecs->systems[sys_id];
        ecs_bitset_t entity_bits = ecs->entities[entity_id].comp_bits;

        if (ecs_entity_system_test(sys->match, sys->comp_bits, entity_bits))
            ecs_sparse_set_remove(&sys->entity_ids, entity_id);
    }
}

/*=============================================================================
 * Internal ID pool functions
 *============================================================================*/

inline static void ecs_id_stack_push(ecs_id_stack_t* stack, ecs_id_t id)
{
    ECS_ASSERT(is_not_null(stack));
    ECS_ASSERT(stack->size < ECS_MAX_ENTITIES);
    stack->array[stack->size++] = id;
}

inline static ecs_id_t ecs_id_stack_pop(ecs_id_stack_t* stack)
{
    ECS_ASSERT(is_not_null(stack));
    return stack->array[--stack->size];
}

inline static int ecs_id_stack_size(ecs_id_stack_t* stack)
{
    return stack->size;
}

/*=============================================================================
 * Internal validation functions
 *============================================================================*/
#ifdef ECS_DEBUG
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

#endif // ECS_DEBUG

#endif // ECS_IMPLEMENTATION

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
