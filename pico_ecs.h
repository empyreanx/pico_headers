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
    It could even be true that a class doesn't neatly fit into the inheritance
    tree at all.

    An ECS solves these problems while also granting more flexibility in
    general. In an ECS there is a clear separation between data (components) and
    logic (systems), which makes it possible to build complex simulations with
    fewer assumptions about how that data will be used. In an ECS it is
    effortless to change functionality, by either adding or removing components
    from entities, and/or by changing system requirements. Adding new logic is
    also simple as well, just defining a new system!

    Please see the examples and unit tests for more details.

    Version 2.4 to 3.0 Migration Guide:
    ----------------------------

    Version 3.0 is a major departure from 2.4. Here is a short guide to help
    make the leap to 3.0.

    1. Make the following substitutions:
        - ecs_register_system  -> ecs_define_system
        - ecs_register_component  -> ecs_define_component
        - ecs_update_system -> ecs_run_system
        - ecs_update_systems -> ecs_run_systems
    2. Remove the 'dt' parameter from system callbacks, replacing it with a
       function call or global variable where necessary.
    3. Replace 'int entity_count' with 'size_t entity_count' in all system
       callbacks
    4. Insert a mask value of 0 into `ecs_define_system` calls, for example,
       `ecs_define_system(ecs, 0, ...)`
    5. Ensure all update calls have the form `ecs_run_system(ctx, sys, 0)`
       and/or `ecs_run_systems(ctx, 0)`
    6. Replace raw IDs with typesafe handles.

    If you encounter any difficultlies with any of these steps and/or your project
    doesn't compile once you're finished, please submit an issue.

    Masks:
    ------

    Masks are a new feature in 3.0. Systems to are assigned to categories (using
    a bitmask) at definition and then can selectively invoke those systems
    at runtime (also using a bitmask).

    Note that passing 0 into `ecs_define_system` means the system matches
    all categories.

    If `ecs_system_t sys = ecs_define_system(ecs, (1 << 0) | (1 << 1), ...)` Then,

    This will run `sys`:

    `ecs_run_system(ecs, sys, (1 << 0) | (1 << 1));`

    And so will this,

    `ecs_run_system(ecs, sys, (1 << 1));`

    But this will not,

    `ecs_run_system(ecs, sys, (1 << 3));`

    Nor will this:

    `ecs_run_system(ecs, sys, 0);`

    Revision History:
    -----------------

    - 3.0 (2025/10/22):
        - Typesafe entity, component, and system handles
        - System category masks
        - More descriptive names for some functions in the public API
        - Invalid entity ID is now 0
        - The 'dt' parameter has been removed
        - Optimizations
        - Some function name changes
        - Significant internal refactoring

    Usage:
    ------

    To use this library in your project, add the following

    > #define PICO_ECS_IMPLEMENTATION
    > #include "pico_ecs.h"

    to a source file (once), then simply include the header normally.

    Macros:
    --------

    - ECS_MALLOC(size, ctx)       (default: malloc)
    - ECS_REALLOC(ptr, size, ctx) (default: realloc)
    - ECS_FREE(ptr, ctx)          (default: free)

    The ctx parameter is sometimes used by custom allocators

    Constants:
    --------

    - PICO_ECS_MAX_COMPONENTS (default: 32)
    - PICO_ECS_MAX_SYSTEMS    (default: 16)

    Must be defined before PICO_ECS_IMPLEMENTATION
*/

#ifndef PICO_ECS_H
#define PICO_ECS_H

#include <stdbool.h> // bool, true, false
#include <stddef.h>  // size_t
#include <stdint.h>  // uint32_t

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ECS context
 */
typedef struct ecs_s ecs_t;

/**
 * @brief Determine ID type
 */
#ifndef ECS_ID_TYPE
#define ECS_ID_TYPE uint64_t
#endif

/**
 * @brief ID used for entity and components
 */
typedef ECS_ID_TYPE ecs_id_t;

/**
 * @brief Determine mask type
 */
#ifndef ECS_MASK_TYPE
#define ECS_MASK_TYPE uint64_t
#endif

/**
 * @brief Type for value used in system matching
 */
typedef ECS_MASK_TYPE ecs_mask_t;

/**
 * @brief Return code for system callback and calling functions
 */
typedef int32_t ecs_ret_t;

/**
 * @brief An entity handle
 */
typedef struct ecs_entity_t { ecs_id_t id; } ecs_entity_t;

/**
 * @brief A component handle
 */
typedef struct ecs_comp_t { ecs_id_t id; } ecs_comp_t;

/**
 * @brief A system handle
 */
typedef struct ecs_system_t { ecs_id_t id; } ecs_system_t;

/**
 * @brief Returns true if the entity is invalid and false otherwise
 */
bool ecs_is_invalid_entity(ecs_entity_t entity);

/**
 * @brief Returns an invalid entity
 */
ecs_entity_t ecs_invalid_entity();

/**
 * @brief Creates an ECS context.
 *
 * @param entity_count The inital number of entities to pre-allocated
 * @param mem_ctx A context for a custom allocator
 *
 * @returns An ECS context or NULL if out of memory
 */
ecs_t* ecs_new(size_t entity_count, void* mem_ctx);

/**
 * @brief Destroys an ECS context
 *
 * @param ecs The ECS context
 */
void ecs_free(ecs_t* ecs);

/**
 * @brief Removes all entities from the ECS, preserving systems and components.
 */
void ecs_reset(ecs_t* ecs);

/**
 * @brief Called when a component is created (via ecs_add)
 *
 * @param ecs    The ECS context
 * @param entity The entity being constructed
 * @param ptr    The pointer to the component
 */
typedef void (*ecs_constructor_fn)(ecs_t* ecs,
                                   ecs_entity_t entity,
                                   void* comp_ptr,
                                   void* args);

/**
 * @brief Called when a component is destroyed (via ecs_remove or ecs_destroy)
 *
 * @param ecs    The ECS context
 * @param entity The entity being destoryed
 * @param ptr    The pointer to the component
 */
typedef void (*ecs_destructor_fn)(ecs_t* ecs,
                                  ecs_entity_t entity,
                                  void* comp_ptr);

/**
 * @brief Defines a component
 *
 * Defines a component with the specfied size in bytes. Components define the
 * game state (usually contained within structs) and are manipulated by systems.
 *
 * @param ecs         The ECS context
 * @param size        The number of bytes to allocate for each component instance
 * @param constructor Called when a component is created (disabled if NULL)
 * @param destructor  Called when a component is destroyed (disabled if NULL)
 * @param udata       Data passed to callbacks (can be NULL)
 * @returns           A component handle
 */
ecs_comp_t ecs_define_component(ecs_t* ecs,
                                size_t size,
                                ecs_constructor_fn constructor,
                                ecs_destructor_fn destructor);

/**
 * @brief System callback
 *
 * Systems implement the core logic of an ECS by manipulating entities
 * and components.
 *
 * @param ecs          The ECS context
 * @param entities     An array of entities managed by the system
 * @param entity_count The number of entities in the array
 * @param udata        The user data associated with the system
 */
typedef ecs_ret_t (*ecs_system_fn)(ecs_t* ecs,
                                   ecs_entity_t* entities,
                                   size_t entity_count,
                                   void* udata);

/**
 * @brief Called when an entity is added to a system
 *
 * @param ecs    The ECS context
 * @param entity The entity being added
 * @param udata  The user data passed to the callback
 */
typedef void (*ecs_added_fn)(ecs_t* ecs, ecs_entity_t entity, void* udata);

/**
 * @brief Called when an entity is removed from a system
 *
 * @param ecs    The ECS context
 * @param entity The enitty being removed
 * @param udata  The user data passed to the callback
 */
typedef void (*ecs_removed_fn)(ecs_t* ecs, ecs_entity_t entity, void* udata);

/**
 * @brief Defines a system
 *
 * Defines a system with the specified parameters. Systems contain the
 * core logic of a game by manipulating game state as defined by components.
 *
 * @param ecs       The ECS context
 * @param mask      Bitmask that determines which categories the system belongs
                    to. A value of 0 matches all categories
 * @param system_cb Callback that is fired every update
 * @param add_cb    Called when an entity is added to the system (can be NULL)
 * @param remove_cb Called when an entity is removed from the system (can be NULL)
 * @param udata     The user data passed to the callbacks
 * @returns         A system handle
 */
ecs_system_t ecs_define_system(ecs_t* ecs,
                               ecs_mask_t mask,
                               ecs_system_fn system_cb,
                               ecs_added_fn add_cb,
                               ecs_removed_fn remove_cb,
                               void* udata);
/**
 * @brief Entities are processed by the target system if they have all of the
 * the components required by the system
 *
 * @param ecs  The ECS context
 * @param sys  The target system
 * @param comp A component to require
 */
void ecs_require_component(ecs_t* ecs, ecs_system_t sys, ecs_comp_t comp);

/**
 * @brief Excludes entities having the specified component from being added to
 * the target system.
 *
 * @param ecs  The ECS context
 * @param sys  The target system
 * @param comp A component to exclude
 */
void ecs_exclude_component(ecs_t* ecs, ecs_system_t sys, ecs_comp_t comp);

/**
 * @brief Enables a system
 *
 * @param ecs    The ECS context
 * @param sys_id The specified system
 */
void ecs_enable_system(ecs_t* ecs, ecs_system_t sys);

/**
 * @brief Disables a system
 *
 * @param ecs The ECS context
 * @param sys The specified system
 */
void ecs_disable_system(ecs_t* ecs, ecs_system_t sys);

/**
 * @brief Updates the callbacks for an existing system
 *
 * @param ecs       The ECS context
 * @param sys       The system
 * @param system_cb Callback that is fired every update
 * @param add_cb    Called when an entity is added to the system (can be NULL)
 * @param remove_cb Called when an entity is removed from the system (can be NULL)
 */
void ecs_set_system_callbacks(ecs_t* ecs,
                              ecs_system_t sys,
                              ecs_system_fn system_cb,
                              ecs_added_fn add_cb,
                              ecs_removed_fn remove_cb);

/**
 * @brief Sets the user data for a system
 *
 * @param ecs   The ECS context
 * @param sys   The system
 * @param udata The user data to set
 */
void ecs_set_system_udata(ecs_t* ecs, ecs_system_t sys, void* udata);

/**
 * @brief Gets the user data from a system
 *
 * @param ecs The ECS context
 * @param sys The system
 * @return    The system's user data
 */
void* ecs_get_system_udata(ecs_t* ecs, ecs_system_t sys);

/**
 * @brief Sets the system's mask
 *
 * @param ecs  The ECS context
 * @param sys  The system
 * @param mask The mask to set
 */
void ecs_set_system_mask(ecs_t* ecs, ecs_system_t sys, ecs_mask_t mask);

/**
 * @brief Returns the system mask
 *
 * @param ecs The ECS context
 * @param sys The system
 * @return    The system's mask
 */
ecs_mask_t ecs_get_system_mask(ecs_t* ecs, ecs_system_t sys);

/**
 * @brief Returns the number of entities assigned to the specified system
 */
size_t ecs_get_system_entity_count(ecs_t* ecs, ecs_system_t sys);

/**
 * @brief Creates an entity
 *
 * @param ecs The ECS context
 *
 * @returns The new entity
 */
ecs_entity_t ecs_create(ecs_t* ecs);

/**
 * @brief Returns true if the entity is currently active and has not been queued
 * for destruction
 *
 * @param ecs The ECS context
 * @param entity The target entity
 */
bool ecs_is_ready(ecs_t* ecs, ecs_entity_t entity);

/**
 * @brief Test if entity has the specified component
 *
 * @param ecs    The ECS context
 * @param entity The entity
 * @param comp   The component
 *
 * @returns True if the entity has the component
 */
bool ecs_has(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp);

/**
 * @brief Adds a component instance to an entity
 *
 * @param ecs    The ECS context
 * @param entity The entity
 * @param comp   The component
 *
 * @returns The component data
 */
void* ecs_add(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp, void* args);

/**
 * @brief Gets a component instance associated with an entity
 *
 * @param ecs    The ECS context
 * @param entity The entity
 * @param comp   The component
 *
 * @returns The component data
 */
void* ecs_get(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp);

/**
 * @brief Destroys an entity
 *
 * Destroys an entity, releasing resources and returning it to the pool.
 *
 * WARNING: This function may change the order of a system's entity array. It
 * should be used with caution. A better option in most circumstances is to use
 * the {@link ecs_queue_destroy} function, which destroys the entity after the
 * system has finished executing.
 *
 * @param ecs    The ECS context
 * @param entity The entity to destroy
 */
void ecs_destroy(ecs_t* ecs, ecs_entity_t entity);

/**
 * @brief Removes a component instance from an entity
 *
 * WARNING: This function may change the order of a system's entity array. It
 * should be used with caution. A better option in most circumstances is to use
 * the {@link ecs_queue_remove} function, which removes the component after the
 * system has finished executing.
 *
 * @param ecs    The ECS context
 * @param entity The entity
 * @param comp   The component
 */
void ecs_remove(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp);

/**
 * @brief Queues an entity for destruction after the current system returns
 *
 * Queued entities are destroyed after the curent iteration.
 *
 * @param ecs    The ECS context
 * @param entity The entity to destroy
 */
void ecs_queue_destroy(ecs_t* ecs, ecs_entity_t entity);

/**
 * @brief Queues a component for removal from the specified entity
 *
 * Queued entity/component pairs that will be deleted after the current system
 * returns.
 *
 * @param ecs    The ECS context
 * @param entity The entity that has the component
 * @param comp   The component to remove
 */
void ecs_queue_remove(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp);

/**
 * @brief Update an individual system
 *
 * Calls system logic on required components, but not excluded ones.
 *
 * @param ecs The ECS context
 * @param sys The system to update
 * @param mask Bitmask that determines which systems run based on category.
 */
ecs_ret_t ecs_run_system(ecs_t* ecs, ecs_system_t sys, ecs_mask_t mask);

/**
 * @brief Updates all systems
 *
 * Calls {@link ecs_run_system} on all components in order of system
 * definition. In many cases it is better to call {@link ecs_run_system} as
 * needed.
 *
 * @param ecs The ECS context
 * @param mask Bitmask that determines which systems run based on category.
 */
ecs_ret_t ecs_run_systems(ecs_t* ecs, ecs_mask_t mask);

#ifdef __cplusplus
}
#endif

#endif // PICO_ECS_H

#ifdef PICO_ECS_IMPLEMENTATION // Define once

#include <stddef.h> // size_t
#include <stdint.h> // uint32_t, uint64_t
#include <stdlib.h> // malloc, realloc, free
#include <string.h> // memcpy, memset

#ifndef PICO_ECS_MAX_COMPONENTS
#define PICO_ECS_MAX_COMPONENTS 32
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

#if !defined(PICO_ECS_MALLOC) || !defined(PICO_ECS_REALLOC) || !defined(PICO_ECS_FREE)
#include <stdlib.h>
#define PICO_ECS_MALLOC(size, ctx)       (malloc(size))
#define PICO_ECS_REALLOC(ptr, size, ctx) (realloc(ptr, size))
#define PICO_ECS_FREE(ptr, ctx)          (free(ptr))
#endif

/*=============================================================================
 *  Aliases
 *============================================================================*/

#define ECS_ASSERT          PICO_ECS_ASSERT
#define ECS_MAX_COMPONENTS  PICO_ECS_MAX_COMPONENTS
#define ECS_MAX_SYSTEMS     PICO_ECS_MAX_SYSTEMS
#define ECS_MALLOC          PICO_ECS_MALLOC
#define ECS_REALLOC         PICO_ECS_REALLOC
#define ECS_FREE            PICO_ECS_FREE

/*=============================================================================
 *  Data structures
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
    size_t        capacity;
    size_t        size;
    size_t*       sparse;
    ecs_entity_t* dense;
} ecs_sparse_set_t;

// A data-structure for providing O(1) operations for working with IDs
typedef struct
{
    size_t    capacity;
    size_t    size; // array size
    ecs_id_t* data;
} ecs_id_array_t;

typedef struct
{
    size_t capacity;
    size_t size; // component size
    void*  data;
} ecs_comp_array_t;

typedef struct
{
    ecs_bitset_t comp_bits;
    bool         active;
    bool         ready;
} ecs_entity_data_t;

typedef struct
{
    ecs_constructor_fn constructor;
    ecs_destructor_fn  destructor;
} ecs_comp_data_t;

typedef struct
{
    bool             active;
    ecs_sparse_set_t entity_ids;
    ecs_mask_t       mask;
    ecs_system_fn    system_cb;
    ecs_added_fn     add_cb;
    ecs_removed_fn   remove_cb;
    ecs_bitset_t     require_bits;
    ecs_bitset_t     exclude_bits;
    void*            udata;
} ecs_sys_data_t;

struct ecs_s
{
    ecs_id_array_t     entity_pool;
    ecs_id_array_t     destroy_queue;
    ecs_id_array_t     remove_queue;
    ecs_entity_data_t* entities;
    size_t             entity_count;
    size_t             next_entity_id;
    ecs_comp_data_t    comps[ECS_MAX_COMPONENTS];
    ecs_comp_array_t   comp_arrays[ECS_MAX_COMPONENTS];
    size_t             comp_count;
    ecs_sys_data_t     systems[ECS_MAX_SYSTEMS];
    size_t             system_count;
    void*              mem_ctx;
};

/*=============================================================================
 * Handle constructors
 *============================================================================*/
static inline ecs_entity_t ecs_make_entity(ecs_id_t id);
static inline ecs_comp_t ecs_make_comp(ecs_id_t id);
static inline ecs_system_t ecs_make_system(ecs_id_t id);

/*=============================================================================
 * Realloc wrapper
 *============================================================================*/
static void* ecs_realloc_zero(ecs_t* ecs, void* ptr, size_t old_size, size_t new_size);

/*=============================================================================
 * Removes entity from ALL systems
 *============================================================================*/
static void ecs_remove_from_systems(ecs_t* ecs, ecs_entity_t entity);

/*=============================================================================
 * Calls destructors on all components of the entity
 *============================================================================*/
static void ecs_destruct(ecs_t* ecs, ecs_id_t entity);

/*=============================================================================
 * Tests if entity is active (created)
 *============================================================================*/
static inline bool ecs_is_active(ecs_t* ecs, ecs_id_t entity_id);

/*=============================================================================
 * Functions to flush destroyed entities and removed component
 *============================================================================*/
static void ecs_flush_destroyed(ecs_t* ecs);
static void ecs_flush_removed(ecs_t* ecs);

/*=============================================================================
 * Bitset functions
 *============================================================================*/
static inline void ecs_bitset_flip(ecs_bitset_t* set, int bit, bool on);
static inline bool ecs_bitset_is_zero(ecs_bitset_t* set);
static inline bool ecs_bitset_test(ecs_bitset_t* set, int bit);
static inline ecs_bitset_t ecs_bitset_and(ecs_bitset_t* set1, ecs_bitset_t* set2);
static inline ecs_bitset_t ecs_bitset_or(ecs_bitset_t* set1, ecs_bitset_t* set2);
static inline ecs_bitset_t ecs_bitset_not(ecs_bitset_t* set);
static inline bool ecs_bitset_equal(ecs_bitset_t* set1, ecs_bitset_t* set2);
static inline bool ecs_bitset_true(ecs_bitset_t* set);

/*=============================================================================
 * Sparse set functions
 *============================================================================*/
static void ecs_sparse_set_init(ecs_t* ecs, ecs_sparse_set_t* set, size_t capacity);
static void ecs_sparse_set_free(ecs_t* ecs, ecs_sparse_set_t* set);
static bool ecs_sparse_set_add(ecs_t* ecs, ecs_sparse_set_t* set, ecs_id_t id);
static bool ecs_sparse_set_find(ecs_sparse_set_t* set, ecs_id_t id, size_t* found);
static bool ecs_sparse_set_remove(ecs_sparse_set_t* set, ecs_id_t id);

/*=============================================================================
 * System entity add/remove functions
 *============================================================================*/
static bool ecs_entity_system_test(ecs_bitset_t* require_bits,
                                   ecs_bitset_t* exclude_bits,
                                   ecs_bitset_t* entity_bits);

/*=============================================================================
 * ID array functions
 *============================================================================*/
static void     ecs_id_array_init(ecs_t* ecs, ecs_id_array_t* pool, int capacity);
static void     ecs_id_array_free(ecs_t* ecs, ecs_id_array_t* pool);
static void     ecs_id_array_push(ecs_t* ecs, ecs_id_array_t* pool, ecs_id_t id);
static ecs_id_t ecs_id_array_pop(ecs_id_array_t* pool);
static int      ecs_id_array_size(ecs_id_array_t* pool);

/*=============================================================================
 * Component array functions
 *============================================================================*/
static void ecs_comp_array_init(ecs_t* ecs, ecs_comp_array_t* array, size_t size, size_t capacity);
static void ecs_comp_array_free(ecs_t* ecs, ecs_comp_array_t* array);
static void ecs_comp_array_resize(ecs_t* ecs, ecs_comp_array_t* array, size_t capacity);

/*=============================================================================
 * Validation functions
 *============================================================================*/
#ifndef NDEBUG
static bool ecs_is_not_null(void* ptr);
static bool ecs_is_valid_component_id(ecs_id_t id);
static bool ecs_is_valid_system_id(ecs_id_t id);
static bool ecs_is_entity_ready(ecs_t* ecs, ecs_id_t entity_id);
static bool ecs_is_component_ready(ecs_t* ecs, ecs_id_t comp_id);
static bool ecs_is_system_ready(ecs_t* ecs, ecs_id_t sys_id);
#endif // NDEBUG

/*=============================================================================
 * Public API implementation
 *============================================================================*/

bool ecs_is_invalid_entity(ecs_entity_t entity)
{
    return 0 == entity.id;
}

ecs_entity_t ecs_invalid_entity()
{
    ecs_entity_t invalid = { 0 };
    return invalid;
}

ecs_t* ecs_new(size_t entity_count, void* mem_ctx)
{
    ECS_ASSERT(entity_count > 0);

    ecs_t* ecs = (ecs_t*)ECS_MALLOC(sizeof(ecs_t), mem_ctx);

    // Out of memory
    if (NULL == ecs)
        return NULL;

    memset(ecs, 0, sizeof(ecs_t));

    ecs->entity_count   = (entity_count > 0) ? entity_count : 1;
    ecs->next_entity_id = 1;
    ecs->mem_ctx        = mem_ctx;

    // Initialize entity pool and queues
    ecs_id_array_init(ecs, &ecs->entity_pool,   entity_count);
    ecs_id_array_init(ecs, &ecs->destroy_queue, entity_count);
    ecs_id_array_init(ecs, &ecs->remove_queue,  entity_count * 2);

    // Allocate entity array
    ecs->entities = (ecs_entity_data_t*)ECS_MALLOC(ecs->entity_count * sizeof(ecs_entity_data_t),
                                                   ecs->mem_ctx);

    // Zero entity array
    memset(ecs->entities, 0, ecs->entity_count * sizeof(ecs_entity_data_t));

    return ecs;
}

void ecs_free(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    for (ecs_id_t entity_id = 0; entity_id < ecs->entity_count; entity_id++)
    {
        if (ecs->entities[entity_id].active)
            ecs_destruct(ecs, entity_id);
    }

    ecs_id_array_free(ecs, &ecs->entity_pool);
    ecs_id_array_free(ecs, &ecs->destroy_queue);
    ecs_id_array_free(ecs, &ecs->remove_queue);

    for (ecs_id_t comp_id = 0; comp_id < ecs->comp_count; comp_id++)
    {
        ecs_comp_array_t* comp_array = &ecs->comp_arrays[comp_id];
        ecs_comp_array_free(ecs, comp_array);
    }

    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++)
    {
        ecs_sys_data_t* sys = &ecs->systems[sys_id];
        ecs_sparse_set_free(ecs, &sys->entity_ids);
    }

    ECS_FREE(ecs->entities, ecs->mem_ctx);
    ECS_FREE(ecs, ecs->mem_ctx);
}

void ecs_reset(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    for (ecs_id_t entity_id = 0; entity_id < ecs->entity_count; entity_id++)
    {
        if (ecs->entities[entity_id].active)
            ecs_destruct(ecs, entity_id);
    }

    ecs->entity_pool.size   = 0;
    ecs->destroy_queue.size = 0;
    ecs->remove_queue.size  = 0;

    memset(ecs->entities, 0, ecs->entity_count * sizeof(ecs_entity_data_t));

    ecs->next_entity_id = 1;

    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++)
    {
        ecs->systems[sys_id].entity_ids.size = 0;
    }
}

ecs_comp_t ecs_define_component(ecs_t* ecs,
                                size_t size,
                                ecs_constructor_fn constructor,
                                ecs_destructor_fn destructor)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs->comp_count < ECS_MAX_COMPONENTS);
    ECS_ASSERT(size > 0);

    ecs_comp_t comp = ecs_make_comp(ecs->comp_count);

    ecs_comp_array_t* comp_array = &ecs->comp_arrays[comp.id];
    ecs_comp_array_init(ecs, comp_array, size, ecs->entity_count);

    ecs->comps[comp.id].constructor = constructor;
    ecs->comps[comp.id].destructor = destructor;

    ecs->comp_count++;

    return comp;
}

ecs_system_t ecs_define_system(ecs_t* ecs,
                               ecs_mask_t mask,
                               ecs_system_fn system_cb,
                               ecs_added_fn add_cb,
                               ecs_removed_fn remove_cb,
                               void* udata)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs->system_count < ECS_MAX_SYSTEMS);
    ECS_ASSERT(NULL != system_cb);

    ecs_system_t sys = ecs_make_system(ecs->system_count);
    ecs_sys_data_t* sys_data = &ecs->systems[sys.id];

    ecs_sparse_set_init(ecs, &sys_data->entity_ids, ecs->entity_count);

    sys_data->active = true;
    sys_data->mask = mask;
    sys_data->system_cb = system_cb;
    sys_data->add_cb = add_cb;
    sys_data->remove_cb = remove_cb;
    sys_data->udata = udata;

    ecs->system_count++;

    return sys;
}

void ecs_require_component(ecs_t* ecs, ecs_system_t sys, ecs_comp_t comp)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys.id));
    ECS_ASSERT(ecs_is_valid_component_id(comp.id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys.id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp.id));

    // Set system component bit for the specified component
    ecs_sys_data_t* sys_data = &ecs->systems[sys.id];
    ecs_bitset_flip(&sys_data->require_bits, comp.id, true);
}

void ecs_exclude_component(ecs_t* ecs, ecs_system_t sys, ecs_comp_t comp)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys.id));
    ECS_ASSERT(ecs_is_valid_component_id(comp.id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys.id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp.id));

    // Set system component bit for the specified component
    ecs_sys_data_t* sys_data = &ecs->systems[sys.id];
    ecs_bitset_flip(&sys_data->exclude_bits, comp.id, true);
}

void ecs_enable_system(ecs_t* ecs, ecs_system_t sys)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys.id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys.id));

    ecs_sys_data_t* sys_data = &ecs->systems[sys.id];
    sys_data->active = true;
}

void ecs_disable_system(ecs_t* ecs, ecs_system_t sys)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys.id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys.id));

    ecs_sys_data_t* sys_data = &ecs->systems[sys.id];
    sys_data->active = false;
}

void ecs_set_system_callbacks(ecs_t* ecs,
                              ecs_system_t sys,
                              ecs_system_fn system_cb,
                              ecs_added_fn add_cb,
                              ecs_removed_fn remove_cb)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys.id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys.id));
    ECS_ASSERT(NULL != system_cb);

    ecs_sys_data_t* sys_data = &ecs->systems[sys.id];
    sys_data->system_cb = system_cb;
    sys_data->add_cb = add_cb;
    sys_data->remove_cb = remove_cb;
}

void ecs_set_system_udata(ecs_t* ecs, ecs_system_t sys, void* udata)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys.id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys.id));

    ecs->systems[sys.id].udata = udata;
}

void* ecs_get_system_udata(ecs_t* ecs, ecs_system_t sys)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys.id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys.id));

    return ecs->systems[sys.id].udata;
}

void ecs_set_system_mask(ecs_t* ecs, ecs_system_t sys, ecs_mask_t mask)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys.id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys.id));

    ecs->systems[sys.id].mask = mask;
}

ecs_mask_t ecs_get_system_mask(ecs_t* ecs, ecs_system_t sys)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys.id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys.id));

    return ecs->systems[sys.id].mask;
}

size_t ecs_get_system_entity_count(ecs_t* ecs, ecs_system_t sys)
{
    return ecs->systems[sys.id].entity_ids.size;
}

ecs_entity_t ecs_create(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    ecs_id_t entity_id = 0;

    // If there is an ID in the pool, pop it
    ecs_id_array_t* pool = &ecs->entity_pool;

    if (0 != ecs_id_array_size(pool))
    {
        entity_id = ecs_id_array_pop(pool);
    }
    else
    {
        // Otherwise, issue a fresh ID
        entity_id = ecs->next_entity_id++;

        // Grow the entities array if necessary
        if (entity_id >= ecs->entity_count)
        {
            size_t old_count = ecs->entity_count;
            size_t new_count = 2 * old_count;

            ecs->entities = (ecs_entity_data_t*)ecs_realloc_zero(ecs, ecs->entities,
                                                                 old_count * sizeof(ecs_entity_data_t),
                                                                 new_count * sizeof(ecs_entity_data_t));

            ecs->entity_count = new_count;
        }
    }

    // Activate the entity and return a handle
    ecs->entities[entity_id].active = true;
    ecs->entities[entity_id].ready  = true;

    return ecs_make_entity(entity_id);
}

bool ecs_is_ready(ecs_t* ecs, ecs_entity_t entity)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    return ecs->entities[entity.id].ready;
}

void ecs_destroy(ecs_t* ecs, ecs_entity_t entity)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_active(ecs, entity.id));

    // Load entity data
    ecs_entity_data_t* entity_data = &ecs->entities[entity.id];

    // Remove entity from systems
    if (ecs_is_ready(ecs, entity))
    {
        ecs_remove_from_systems(ecs, entity);
    }

    // Call destructors on entity components
    ecs_destruct(ecs, entity.id);

    // Push entity ID into pool
    ecs_id_array_t* pool = &ecs->entity_pool;
    ecs_id_array_push(ecs, pool, entity.id);

    // Reset entity (sets bitset to 0 and, active and ready to false)
    memset(entity_data, 0, sizeof(ecs_entity_data_t));
}

bool ecs_has(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_component_id(comp.id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity.id));

    // Load entity data
    ecs_entity_data_t* entity_data = &ecs->entities[entity.id];

    // Return true if the component belongs to the entity
    return ecs_bitset_test(&entity_data->comp_bits, comp.id);
}

void* ecs_get(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_component_id(comp.id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp.id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity.id));

    // Return pointer to component
    //  eid0,  eid1   eid2, ...
    // [comp0, comp1, comp2, ...]
    ecs_comp_array_t* comp_array = &ecs->comp_arrays[comp.id];
    return (char*)comp_array->data + (comp_array->size * entity.id);
}

void* ecs_add(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp, void* args)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_component_id(comp.id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity.id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp.id));

    // Load entity data
    ecs_entity_data_t* entity_data = &ecs->entities[entity.id];

    // Load component
    ecs_comp_array_t* comp_array = &ecs->comp_arrays[comp.id];
    ecs_comp_data_t* comp_data = &ecs->comps[comp.id];

    // Grow the component array
    ecs_comp_array_resize(ecs, comp_array, entity.id);

    // Get pointer to component
    void* comp_ptr = ecs_get(ecs, entity, comp);

    // Zero component
    memset(comp_ptr, 0, comp_array->size);

    // Call constructor
    if (comp_data->constructor)
        comp_data->constructor(ecs, entity, comp_ptr, args);

    // Set entity component bit that determines which systems this entity
    // belongs to
    ecs_bitset_flip(&entity_data->comp_bits, comp.id, true);

    // Add or remove entity from systems
    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++)
    {
        ecs_sys_data_t* sys_data = &ecs->systems[sys_id];

        if (ecs_entity_system_test(&sys_data->require_bits, &sys_data->exclude_bits, &entity_data->comp_bits))
        {
            if (ecs_sparse_set_add(ecs, &sys_data->entity_ids, entity.id))
            {
                if (sys_data->add_cb)
                    sys_data->add_cb(ecs, entity, sys_data->udata);
            }
        }
        else // Just remove the entity if its components no longer match for whatever reason.
        {
            if (!ecs_bitset_is_zero(&sys_data->exclude_bits) &&
                 ecs_sparse_set_remove(&sys_data->entity_ids, entity.id))
            {
                if (sys_data->remove_cb)
                    sys_data->remove_cb(ecs, entity, sys_data->udata);
            }
        }
    }

    // Return component
    return comp_ptr;
}

void ecs_remove(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_component_id(comp.id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp.id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity.id));

    // Load entity data
    ecs_entity_data_t* entity_data = &ecs->entities[entity.id];

    // Create bit mask with comp bit flipped on
    ecs_bitset_t comp_bit;

    memset(&comp_bit, 0, sizeof(ecs_bitset_t));
    ecs_bitset_flip(&comp_bit, comp.id, true);

    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++)
    {
        ecs_sys_data_t* sys_data = &ecs->systems[sys_id];

        if (ecs_entity_system_test(&sys_data->require_bits, &sys_data->exclude_bits, &comp_bit))
        {
            if (ecs_sparse_set_remove(&sys_data->entity_ids, entity.id))
            {
                if (sys_data->remove_cb)
                    sys_data->remove_cb(ecs, entity, sys_data->udata);
            }
        }
        else
        {
            if (!ecs_bitset_is_zero(&sys_data->exclude_bits) &&
                 ecs_sparse_set_add(ecs, &sys_data->entity_ids, entity.id))
            {
                if (sys_data->add_cb)
                    sys_data->add_cb(ecs, entity, sys_data->udata);
            }
        }
    }

    ecs_comp_data_t* comp_data = &ecs->comps[comp.id];

    if (comp_data->destructor)
    {
        void* comp_ptr = ecs_get(ecs, entity, comp);
        comp_data->destructor(ecs, entity, comp_ptr);
    }

    // Reset the relevant component mask bit
    ecs_bitset_flip(&entity_data->comp_bits, comp.id, false);
}

void ecs_queue_destroy(ecs_t* ecs, ecs_entity_t entity)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity.id));

    ecs_remove_from_systems(ecs, entity);

    ecs->entities[entity.id].ready = false;

    ecs_id_array_push(ecs, &ecs->destroy_queue, entity.id);
}

void ecs_queue_remove(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity.id));
    ECS_ASSERT(ecs_has(ecs, entity, comp));

    ecs_id_array_push(ecs, &ecs->remove_queue, entity.id);
    ecs_id_array_push(ecs, &ecs->remove_queue, comp.id);
}

ecs_ret_t ecs_run_system(ecs_t* ecs, ecs_system_t sys, ecs_mask_t mask)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys.id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys.id));

    ecs_sys_data_t* sys_data = &ecs->systems[sys.id];

    if (!sys_data->active)
        return 0;

    if (0 != sys_data->mask && !(sys_data->mask & mask))
        return 0;

    ecs_ret_t code = sys_data->system_cb(ecs,
                     sys_data->entity_ids.dense,
                     sys_data->entity_ids.size,
                     sys_data->udata);

    ecs_flush_destroyed(ecs);
    ecs_flush_removed(ecs);

    return code;
}

ecs_ret_t ecs_run_systems(ecs_t* ecs, ecs_mask_t mask)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++)
    {
        ecs_system_t sys = ecs_make_system(sys_id);
        ecs_ret_t code = ecs_run_system(ecs, sys, mask);

        if (0 != code)
            return code;
    }

    return 0;
}

/*=============================================================================
 * Handle constructors
 *============================================================================*/
static inline ecs_entity_t ecs_make_entity(ecs_id_t id)
{
    ecs_entity_t entity = { id };
    return entity;
}

static inline ecs_comp_t ecs_make_comp(ecs_id_t id)
{
    ecs_comp_t comp = { id };
    return comp;
}

static inline ecs_system_t ecs_make_system(ecs_id_t id)
{
    ecs_system_t sys = { id };
    return sys;
}

/*=============================================================================
 * Realloc wrapper
 *============================================================================*/
static void* ecs_realloc_zero(ecs_t* ecs, void* ptr, size_t old_size, size_t new_size)
{
    (void)ecs;

    ptr = ECS_REALLOC(ptr, new_size, ecs->mem_ctx);

    if (new_size > old_size && ptr) {
        size_t diff = new_size - old_size;
        void* start = ((char*)ptr)+ old_size;
        memset(start, 0, diff);
    }

    return ptr;
}

/*=============================================================================
 * Tests if entity is active (created)
 *============================================================================*/
static inline bool ecs_is_active(ecs_t* ecs, ecs_id_t entity_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    return ecs->entities[entity_id].active;
}

/*=============================================================================
 * Calls destructors on all components of the entity
 *============================================================================*/
static void ecs_destruct(ecs_t* ecs, ecs_id_t entity_id)
{
    // Load entity
    ecs_entity_data_t* entity = &ecs->entities[entity_id];

    // Loop through components and call the destructors
    for (ecs_id_t comp_id = 0; comp_id < ecs->comp_count; comp_id++)
    {
        if (ecs_bitset_test(&entity->comp_bits, comp_id))
        {
            ecs_comp_data_t* comp = &ecs->comps[comp_id];

            if (comp->destructor)
            {
                // Get component pointer directly without ecs_get to avoid
                // ready assertion, since entity may be queued for destruction
                ecs_comp_array_t* comp_array = &ecs->comp_arrays[comp_id];
                void* comp_ptr = (char*)comp_array->data + (comp_array->size * entity_id);
                ecs_entity_t entity = ecs_make_entity(entity_id);

                comp->destructor(ecs, entity, comp_ptr);
            }
        }
    }
}

/*=============================================================================
 * Removes entity from ALL systems
 *============================================================================*/
static void ecs_remove_from_systems(ecs_t* ecs, ecs_entity_t entity)
{
    // Load entity
    ecs_entity_data_t* entity_data = &ecs->entities[entity.id];

    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++)
    {
        ecs_sys_data_t* sys_data = &ecs->systems[sys_id];

        if (ecs_entity_system_test(&sys_data->require_bits,
                                   &sys_data->exclude_bits,
                                   &entity_data->comp_bits) &&
            ecs_sparse_set_remove(&sys_data->entity_ids, entity.id))
        {
            if (sys_data->remove_cb)
                sys_data->remove_cb(ecs, entity, sys_data->udata);
        }
    }
}

/*=============================================================================
 * Functions to flush destroyed entity and removed component
 *============================================================================*/

static void ecs_flush_destroyed(ecs_t* ecs)
{
    ecs_id_array_t* destroy_queue = &ecs->destroy_queue;

    for (size_t i = 0; i < destroy_queue->size; i++)
    {
        ecs_id_t entity_id = destroy_queue->data[i];

        if (ecs_is_active(ecs, entity_id))
            ecs_destroy(ecs, ecs_make_entity(entity_id));
    }

    destroy_queue->size = 0;
}

static void ecs_flush_removed(ecs_t* ecs)
{
    ecs_id_array_t* remove_queue = &ecs->remove_queue;

    for (size_t i = 0; i < remove_queue->size; i += 2)
    {
        ecs_id_t entity_id = remove_queue->data[i];

        if (ecs_is_active(ecs, entity_id))
        {
            ecs_id_t comp_id = remove_queue->data[i + 1];
            ecs_remove(ecs, ecs_make_entity(entity_id), ecs_make_comp(comp_id));
        }
    }

    remove_queue->size = 0;
}


/*=============================================================================
 * Bitset functions
 *============================================================================*/

#if ECS_MAX_COMPONENTS <= 64

static inline bool ecs_bitset_is_zero(ecs_bitset_t* set)
{
    return *set == 0;
}

static inline void ecs_bitset_flip(ecs_bitset_t* set, int bit, bool on)
{
    if (on)
        *set |=  ((uint64_t)1 << bit);
    else
        *set &= ~((uint64_t)1 << bit);
}

static inline bool ecs_bitset_test(ecs_bitset_t* set, int bit)
{
    return *set & ((uint64_t)1 << bit);
}

static inline ecs_bitset_t ecs_bitset_and(ecs_bitset_t* set1, ecs_bitset_t* set2)
{
    return *set1 & *set2;
}

static inline ecs_bitset_t ecs_bitset_or(ecs_bitset_t* set1, ecs_bitset_t* set2)
{
    return *set1 | *set2;
}

static inline ecs_bitset_t ecs_bitset_not(ecs_bitset_t* set)
{
    return ~(*set);
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

static inline bool ecs_bitset_is_zero(ecs_bitset_t* set)
{
    for (int i = 0; i < ECS_BITSET_SIZE; i++)
    {
        if (set->array[i] != 0)
            return false;
    }

    return true;
}

static inline void ecs_bitset_flip(ecs_bitset_t* set, int bit, bool on)
{
    int index = bit / ECS_BITSET_WIDTH;

    if (on)
        set->array[index] |=  ((uint64_t)1 << bit % ECS_BITSET_WIDTH);
    else
        set->array[index] &= ~((uint64_t)1 << bit % ECS_BITSET_WIDTH);
}

static inline bool ecs_bitset_test(ecs_bitset_t* set, int bit)
{
    int index = bit / ECS_BITSET_WIDTH;
    return set->array[index] & ((uint64_t)1 << bit % ECS_BITSET_WIDTH);
}

static inline ecs_bitset_t ecs_bitset_and(ecs_bitset_t* set1,
                                          ecs_bitset_t* set2)
{
    ecs_bitset_t set;

    for (int i = 0; i < ECS_BITSET_SIZE; i++)
    {
        set.array[i] = set1->array[i] & set2->array[i];
    }

    return set;
}

static inline ecs_bitset_t ecs_bitset_or(ecs_bitset_t* set1,
                                         ecs_bitset_t* set2)
{
    ecs_bitset_t set;

    for (int i = 0; i < ECS_BITSET_SIZE; i++)
    {
        set.array[i] = set1->array[i] | set2->array[i];
    }

    return set;
}

static inline ecs_bitset_t ecs_bitset_not(ecs_bitset_t* set)
{
    ecs_bitset_t out;

    for (int i = 0; i < ECS_BITSET_SIZE; i++)
    {
        out.array[i] = ~set->array[i];
    }

    return out;
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
 * Sparse set functions
 *============================================================================*/

static void ecs_sparse_set_init(ecs_t* ecs, ecs_sparse_set_t* set, size_t capacity)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(set));
    ECS_ASSERT(capacity > 0);

    (void)ecs;

    set->capacity = capacity;
    set->size = 0;

    set->dense  = (ecs_entity_t*)ECS_MALLOC(capacity * sizeof(ecs_id_t), ecs->mem_ctx);
    set->sparse = (size_t*)      ECS_MALLOC(capacity * sizeof(size_t),   ecs->mem_ctx);

    memset(set->sparse, 0, capacity * sizeof(size_t));
}

static void ecs_sparse_set_free(ecs_t* ecs, ecs_sparse_set_t* set)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(set));

    (void)ecs;

    ECS_FREE(set->dense,  ecs->mem_ctx);
    ECS_FREE(set->sparse, ecs->mem_ctx);
}

static bool ecs_sparse_set_add(ecs_t* ecs, ecs_sparse_set_t* set, ecs_id_t id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(set));

    (void)ecs;

    // Grow sparse set if necessary
    if (id >= set->capacity)
    {
        size_t old_capacity = set->capacity;
        size_t new_capacity = old_capacity;

        // Calculate new capacity
        new_capacity *= 2;

        // Grow dense array
        set->dense = (ecs_entity_t*)ECS_REALLOC(set->dense,
                                                new_capacity * sizeof(ecs_id_t),
                                                ecs->mem_ctx);

        // Grow sparse array and zero it
        set->sparse = (size_t*)ecs_realloc_zero(ecs,
                                                set->sparse,
                                                old_capacity * sizeof(size_t),
                                                new_capacity * sizeof(size_t));

        // Set the new capacity
        set->capacity = new_capacity;
    }

    // Check if ID exists within the set
    if (ecs_sparse_set_find(set, id, NULL))
        return false;

    // Add ID to set
    set->dense[set->size].id = id;
    set->sparse[id] = set->size;

    set->size++;

    return true;
}

static bool ecs_sparse_set_find(ecs_sparse_set_t* set, ecs_id_t id, size_t* found)
{
    ECS_ASSERT(ecs_is_not_null(set));

    if (set->sparse[id] < set->size && set->dense[set->sparse[id]].id == id)
    {
        if (found) *found = set->sparse[id];
        return true;
    }
    else
    {
        if (found) *found = 0;
        return false;
    }
}

static bool ecs_sparse_set_remove(ecs_sparse_set_t* set, ecs_id_t id)
{
    ECS_ASSERT(ecs_is_not_null(set));

    if (!ecs_sparse_set_find(set, id, NULL))
        return false;

    // Swap and remove (changes order of array)
    ecs_id_t tmp = set->dense[set->size - 1].id;
    set->dense[set->sparse[id]].id = tmp;
    set->sparse[tmp] = set->sparse[id];

    set->size--;

    return true;
}

/*=============================================================================
 * System entity add/remove functions
 *============================================================================*/

inline static bool ecs_entity_system_test(ecs_bitset_t* require_bits,
                                          ecs_bitset_t* exclude_bits,
                                          ecs_bitset_t* entity_bits)
{
    if (!ecs_bitset_is_zero(exclude_bits))
    {
        ecs_bitset_t overlap = ecs_bitset_and(entity_bits, exclude_bits);

        if (ecs_bitset_true(&overlap))
        {
            return false;
        }
    }

    ecs_bitset_t entity_and_require = ecs_bitset_and(entity_bits, require_bits);
    return ecs_bitset_equal(&entity_and_require, require_bits);
}

/*=============================================================================
 * ID array functions
 *============================================================================*/

inline static void ecs_id_array_init(ecs_t* ecs, ecs_id_array_t* array, int capacity)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(array));
    ECS_ASSERT(capacity > 0);

    (void)ecs;

    array->size = 0;
    array->capacity = capacity;
    array->data = (ecs_id_t*)ECS_MALLOC(capacity * sizeof(ecs_id_t), ecs->mem_ctx);
}

inline static void ecs_id_array_free(ecs_t* ecs, ecs_id_array_t* array)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(array));

    (void)ecs;

    ECS_FREE(array->data, ecs->mem_ctx);
}

inline static void ecs_id_array_push(ecs_t* ecs, ecs_id_array_t* array, ecs_id_t id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(array));
    ECS_ASSERT(array->capacity > 0);

    (void)ecs;

    if (array->size == array->capacity)
    {
        array->capacity *= 2;
        array->data = (ecs_id_t*)ECS_REALLOC(array->data,
                                              array->capacity * sizeof(ecs_id_t),
                                              ecs->mem_ctx);
    }

    array->data[array->size++] = id;
}

inline static ecs_id_t ecs_id_array_pop(ecs_id_array_t* array)
{
    ECS_ASSERT(ecs_is_not_null(array));
    return array->data[--array->size];
}

inline static int ecs_id_array_size(ecs_id_array_t* array)
{
    return array->size;
}

static void ecs_comp_array_init(ecs_t* ecs, ecs_comp_array_t* array, size_t size, size_t capacity)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(array));

    (void)ecs;

    memset(array, 0, sizeof(ecs_comp_array_t));

    array->capacity = capacity;
    array->size = size;
    array->data = ECS_MALLOC(size * capacity, ecs->mem_ctx);
}

static void ecs_comp_array_free(ecs_t* ecs, ecs_comp_array_t* array)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(array));

    (void)ecs;

    ECS_FREE(array->data, ecs->mem_ctx);
}

static void ecs_comp_array_resize(ecs_t* ecs, ecs_comp_array_t* array, size_t capacity)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(array));

    (void)ecs;

    if (capacity >= array->capacity)
    {
        array->capacity *= 2;
        array->data = ECS_REALLOC(array->data,
                                  array->capacity * array->size,
                                  ecs->mem_ctx);
    }
}

/*=============================================================================
 * Validation functions
 *============================================================================*/
#ifndef NDEBUG
static bool ecs_is_not_null(void* ptr)
{
    return NULL != ptr;
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
    return comp_id < ecs->comp_count;
}

static bool ecs_is_system_ready(ecs_t* ecs, ecs_id_t sys_id)
{
    return sys_id < ecs->system_count;
}

#endif // NDEBUG

#endif // PICO_ECS_IMPLEMENTATION

/*
    ----------------------------------------------------------------------------
    This software is available under two licenses (A) or (B). You may choose
    either one as you wish:
    ----------------------------------------------------------------------------

    (A) The zlib License

    Copyright (c) 2025 James McLean

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
