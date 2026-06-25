/**
    @file pico_ecs.h
    @brief A pure and simple ECS

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Features:
    ---------
    - Written in C11
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

    - 3.1 (2026/02/01):
        - Fixed sparse set related bugs
        - More sophisticated logic regarding adding entities to/from systems
        - Functions ecs_queue_remove and ecs_queue_destroy have been removed.
          (they can be replaced directly by ecs_remove and ecs_destroy
          respectively)
        - Improved unit test quality and coverage

    - 3.2 (2026/03/09):
        - Capacity overflow detection
        - Invalid entity ID value has been reverted to max ID value

    - 3.3 (2026/06/05):
        - New command queue that ensures add/remove/set/destroy operations are
          performed in the order the corresponding functions were called.
        - Pointers obtained by ecs_get are now stable.
        - The ecs_add constructor/destructor callbacks have been replaced by
          on_add/on_remove callbacks.
        - A new function 'ecs_set' has been added. If the entity does not have
          the component it is added first, then the component's value is set.
          If called during system iteration, then setting the value is deferred
          until after the system completes. This function effectively replaces
          the old ecs_add/constructor functionality.
        - ecs_require_component and ecs_exclude_component have been renamed to
          ecs_require and ecs_exclude.
        - Renamed ecs_get_system_entity_count to ecs_get_entity_count.
        - Added ecs_get_entity_array that return the entities associated with a
          system.

    - 3.4 (2026/06/18):
        - ecs_add now accepts an optional 'args' pointer that is forwarded to the
          on_add callback. When the component is defined with a non-zero
          args_size, the args are copied into the command arena so the caller
          need not keep them alive (relevant for deferred adds during system
          iteration).
        - Components may now specify a default_value that is copied into the
          component on add.

    - 3.5 (2026/06/21):
        - Added a publish/subscribe event system: ecs_define_event,
          ecs_subscribe, ecs_unsubscribe, ecs_emit, and ecs_dispatch. Events are
          queued when emitted and delivered to subscribers only when
          ecs_dispatch is called (never automatically).

    - 3.6 (2026/06/24):
        - The per-system on_join/on_leave callbacks have been removed and
          replaced by two built-in events. When an entity joins (or leaves) a
          system the ECS emits a join (or leave) event whose source is the
          system and whose payload is the entity. Because these events are
          queued like any other, the callbacks now fire on ecs_dispatch rather
          than synchronously.
        - Events now carry an optional source id. Emit with a source using
          ecs_emit_from, and scope a listener to a single source with
          ecs_subscribe_to. A plain ecs_subscribe still receives events from
          every source. This is the infrastructure that ties a join/leave
          listener to a specific system; see ecs_on_join and ecs_on_leave.
        - ecs_set_system_callbacks has been renamed to ecs_set_system_callback
          and now updates only the system callback.

    - 3.7 (2026/06/24):
        - The component on_add/on_remove/on_set callbacks have been removed from
          ecs_comp_desc_t and are now registered with ecs_on_add, ecs_on_remove,
          and ecs_on_set. Like the system join/leave callbacks, they are
          delivered as built-in events and therefore fire on ecs_dispatch rather
          than synchronously. In particular, on_add no longer initializes a
          component during ecs_add; the component is zeroed/defaulted
          immediately, but the on_add callback runs on the next dispatch.

    - 3.8 (2026/06/24):
        - Events now support two delivery modes. ecs_emit/ecs_emit_from now
          deliver synchronously, invoking matching listeners immediately. A new
          ecs_enqueue/ecs_enqueue_from family appends to the event queue for
          deferred delivery on ecs_dispatch (the previous ecs_emit behavior).
          The built-in lifecycle events are enqueued, so on_add/on_remove/on_set
          and join/leave continue to fire on ecs_dispatch.

    - 3.9 (2026/06/24):
        - The system join/leave events are now wired like the component events:
          a single forwarding listener per event routes to the right system's
          callback, instead of one scoped subscription per system. As a result
          the join/leave payload now carries the entity and the system rather
          than a bare entity, and systems no longer consume a listener slot each
          (decoupling them from PICO_ECS_MAX_LISTENERS). The ecs_on_join/
          ecs_on_leave API is unchanged.
        - Added ecs_get_add_event/ecs_get_remove_event/ecs_get_set_event so the
          built-in component events can be observed via a raw ecs_subscribe,
          like the join/leave events. A raw subscriber receives these events
          even for systems/components with no convenience callback registered.
          The event payload layouts are internal to the library.

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
    - ECS_MEMSET                  (default: memset)
    - ECS_MEMCPY                  (default: memcpy)

    The ctx parameter is sometimes used by custom allocators

    Constants:
    --------

    - PICO_ECS_MAX_COMPONENTS  (default: 32)
    - PICO_ECS_MAX_SYSTEMS     (default: 16)
    - PICO_ECS_COMP_BLOCK_SIZE (default: 64)
    - PICO_ECS_MAX_EVENTS      (default: 32)
    - PICO_ECS_MAX_LISTENERS   (default: 16)

    Must be defined before PICO_ECS_IMPLEMENTATION

    Events:
    -------

    Events are a publish/subscribe message bus that decouples the code that
    detects something from the code that reacts to it.

    An event type is declared with `ecs_define_event`, specifying the size of
    its payload (which may be 0). Listeners register with `ecs_subscribe` and
    are removed with `ecs_unsubscribe`.

    There are two ways to deliver an event:

    - Synchronous: `ecs_emit` delivers the event immediately, invoking every
      matching listener before it returns. The payload is not copied.

    - Asynchronous: `ecs_enqueue` appends the event to a queue and copies its
      payload (so the caller need not keep it alive). Queued events are
      delivered only when `ecs_dispatch` is called, never automatically.
      Enqueuing is the safe choice from within a running system, since delivery
      is deferred until the system completes.

    Events may also carry a source id. `ecs_emit_from` / `ecs_enqueue_from` tag
    an event with a source, and `ecs_subscribe_to` scopes a listener so it only
    receives events from a particular source. A listener registered with
    `ecs_subscribe` receives events regardless of source. The ECS uses this
    mechanism for the built-in lifecycle events (see below), whose source is the
    system or component the event concerns.

    The component lifecycle callbacks (on_add/on_remove/on_set) and the system
    join/leave callbacks are delivered through built-in events. These are
    enqueued, so they fire on `ecs_dispatch` rather than synchronously.

    System join/leave:
    ------------------

    When an entity starts matching a system it "joins" that system; when it
    stops matching (or is destroyed) it "leaves". The ECS emits a built-in join
    or leave event for each such transition. The event's source is the system
    and its payload carries the entity and the system. Use
    `ecs_on_join`/`ecs_on_leave` for a per-system callback (which receives the
    entity directly), or subscribe to `ecs_get_join_event`/`ecs_get_leave_event`
    directly to observe every system. These events are enqueued, so they are
    delivered on `ecs_dispatch`.

    Because the built-in lifecycle events are enqueued, they are delivered only
    when `ecs_dispatch` is called (never automatically) and are safe to produce
    while a system is iterating (mirroring the deferred command queue). The
    listeners then run outside of system iteration, so component operations they
    perform take effect immediately. Events enqueued by a listener during
    dispatch are delivered within the same `ecs_dispatch` call.
*/

#ifndef PICO_ECS_H
#define PICO_ECS_H

#include <stdbool.h> // bool, true, false
#include <stddef.h>  // size_t
#include <stdint.h>  // uint32_t, uint64_t
#include <limits.h>  // SIZE_MAX

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ECS context
 */
typedef struct ecs_s ecs_t;

/**
 * @brief Determine ID type. It should be unsigned.
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
 * @brief An event handle
 */
typedef struct ecs_event_t { ecs_id_t id; } ecs_event_t;

/**
 * @brief An invalid ID
 */
#define ECS_INVALID_ID ((ecs_id_t)-1)

/**
 * @brief True if the argument (entity/system/component) is invalid
 */
#define ECS_IS_INVALID(obj) ((obj.id) == ECS_INVALID_ID)

/**
 * @brief Creates an ECS context.
 *
 * @param entity_count The inital number of entities to pre-allocated
 * @param mem_ctx A context for a custom allocator
 *
 * @returns An ECS context or NULL if out of memory
 */
ecs_t* ecs_new(size_t entity_capacity, void* mem_ctx);

/**
 * @brief Destroys an ECS context
 *
 * @param ecs The ECS context
 */
void ecs_free(ecs_t* ecs);

/**
 * @brief Removes all entities from the ECS, preserving systems, components, and
 * event subscriptions. Any pending (undispatched) events are discarded.
 */
void ecs_reset(ecs_t* ecs);

/**
 * @brief Optional parameters for component definition
 *
 * To react to a component being added, removed, or set, register a callback
 * with {@link ecs_on_add} / {@link ecs_on_remove} / {@link ecs_on_set} after
 * defining the component.
 *
 * @param default_value Optional initial component value, copied on add (can be NULL)
 * @param args_size    Size, in bytes, of the args buffer passed to ecs_add. The
 *                     args are copied (this many bytes) and delivered to the
 *                     on_add callback, so the caller need not keep them alive.
 *                     Leave 0 if the component takes no args.
 * @param udata        User data passed to the component callbacks (can be NULL)
 */
typedef struct
{
    void* default_value;
    size_t args_size;
    void* udata;
} ecs_comp_desc_t;

/**
 * @brief Defines a component
 *
 * Defines a component with the specified size in bytes. Components define the
 * game state (usually contained within structs) and are manipulated by systems.
 *
 * @param ecs  The ECS context
 * @param size The number of bytes to allocate for each component instance
 * @param def   Optional parameters for callbacks and user data (can be NULL)
 * @returns     A component handle
 */
ecs_comp_t ecs_define_component(ecs_t* ecs,
                                size_t size,
                                const ecs_comp_desc_t* desc);

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
 * @brief Optional parameters for system definition
 *
 * To react to entities joining or leaving the system, subscribe a listener
 * with {@link ecs_on_join} / {@link ecs_on_leave} after defining the system.
 *
 * @param mask        Bitmask that assigns the system to one or more categories.
 *                    A value of 0 means the system matches all categories.
 * @param udata       User data passed to the system callback (can be NULL)
 */
typedef struct
{
    ecs_mask_t mask;
    void* udata;
} ecs_sys_desc_t;

/**
 * @brief Defines a system
 *
 * Defines a system with the specified parameters. Systems contain the
 * core logic of a game by manipulating game state as defined by components.
 *
 * @param ecs       The ECS context
 * @param system_cb Callback that is fired every update
 * @param def       Optional parameters for mask, join/leave callbacks, and user data (can be NULL)
 * @returns         A system handle
 */
ecs_system_t ecs_define_system(ecs_t* ecs,
                               ecs_system_fn system_cb,
                               const ecs_sys_desc_t* desc);


/**
 * @brief Entities are processed by the target system if they have all of the
 * the components required by the system
 *
 * @param ecs  The ECS context
 * @param sys  The target system
 * @param comp A component to require
 */
void ecs_require(ecs_t* ecs, ecs_system_t sys, ecs_comp_t comp);

/**
 * @brief Excludes entities having the specified component from being added to
 * the target system.
 *
 * @param ecs  The ECS context
 * @param sys  The target system
 * @param comp A component to exclude
 */
void ecs_exclude(ecs_t* ecs, ecs_system_t sys, ecs_comp_t comp);

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
 * @brief Updates the callback for an existing system
 *
 * @param ecs       The ECS context
 * @param sys       The system
 * @param system_cb Callback that is fired every update
 */
void ecs_set_system_callback(ecs_t* ecs,
                             ecs_system_t sys,
                             ecs_system_fn system_cb);

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
 * @brief Returns the entities associated with the specified system
 */
ecs_entity_t* ecs_get_entity_array(ecs_t* ecs, ecs_system_t sys);

/**
 * @brief Returns the number of entities assigned to the specified system
 */
size_t ecs_get_entity_count(ecs_t* ecs, ecs_system_t sys);

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
 * @param args   Optional arguments passed to the component constructor. When
 *               the add is deferred (called from within a running system) and
 *               the component was defined with a non-zero args_size, args_size
 *               bytes are copied and stored until the command queue is flushed,
 *               so the caller need not keep the args alive. If args_size is 0,
 *               no copy is made and NULL is forwarded to the constructor.
 *
 * @returns The component data
 */
void ecs_add(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp, void* args);

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
 * @brief Copies data into a component instance associated with an entity
 *
 * If the entity does not have the component this adds the component to the
 * enity. If called during system iteration the operation is deferred until
 * after the system completes.
 *
 * @param ecs    The ECS context
 * @param entity The entity
 * @param comp   The component
 * @param data   Pointer to the data to copy into the component
 */
void ecs_set(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp, void* data);

/**
 * @brief Destroys an entity
 *
 * Destroys an entity, releasing resources and returning it to the pool.
 *
 * @param ecs    The ECS context
 * @param entity The entity to destroy
 */
void ecs_destroy(ecs_t* ecs, ecs_entity_t entity);

/**
 * @brief Removes a component instance from an entity
 *
 * @param ecs    The ECS context
 * @param entity The entity
 * @param comp   The component
 */
void ecs_remove(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp);

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

/**
 * @brief Event listener callback
 *
 * Called for each subscribed listener when a matching event is delivered,
 * whether synchronously via {@link ecs_emit} or from the queue via
 * {@link ecs_dispatch}.
 *
 * @param ecs          The ECS context
 * @param event        The event being delivered
 * @param payload      Pointer to the event payload, or NULL if the event was
 *                     defined with a payload size of 0. The payload is only
 *                     valid for the duration of the call.
 * @param payload_size The size, in bytes, of the payload
 * @param udata        The user data supplied to {@link ecs_subscribe}
 */
typedef void (*ecs_listener_fn)(ecs_t* ecs,
                                ecs_event_t event,
                                const void* payload,
                                size_t payload_size,
                                void* udata);

/**
 * @brief Defines an event type
 *
 * Events are messages delivered to subscribed listeners. They are decoupled
 * from components and entities and may carry an arbitrary, fixed-size payload.
 *
 * @param ecs          The ECS context
 * @param payload_size The number of bytes carried by each instance of the
 *                     event. May be 0 for events that carry no payload.
 * @returns            An event handle
 */
ecs_event_t ecs_define_event(ecs_t* ecs, size_t payload_size);

/**
 * @brief Subscribes a listener to an event
 *
 * @param ecs   The ECS context
 * @param event The event to subscribe to
 * @param fn    The listener callback to invoke when the event is delivered
 * @param udata User data passed to the listener (can be NULL)
 * @returns     A subscription handle that can be passed to
 *              {@link ecs_unsubscribe}
 */
ecs_id_t ecs_subscribe(ecs_t* ecs,
                       ecs_event_t event,
                       ecs_listener_fn fn,
                       void* udata);

/**
 * @brief Subscribes a listener to an event, scoped to a single source
 *
 * The listener is invoked only for events emitted with a matching source (see
 * {@link ecs_emit_from}). Events emitted with no source (via {@link ecs_emit})
 * are not delivered to a scoped listener. This is how a listener is tied to a
 * specific system for the built-in join/leave events.
 *
 * @param ecs    The ECS context
 * @param event  The event to subscribe to
 * @param source The source id to match
 * @param fn     The listener callback to invoke
 * @param udata  User data passed to the listener (can be NULL)
 * @returns      A subscription handle that can be passed to
 *               {@link ecs_unsubscribe}
 */
ecs_id_t ecs_subscribe_to(ecs_t* ecs,
                          ecs_event_t event,
                          ecs_id_t source,
                          ecs_listener_fn fn,
                          void* udata);

/**
 * @brief Removes a listener from an event
 *
 * @param ecs   The ECS context
 * @param event The event to unsubscribe from
 * @param sub   The subscription handle returned by {@link ecs_subscribe}
 */
void ecs_unsubscribe(ecs_t* ecs, ecs_event_t event, ecs_id_t sub);

/**
 * @brief Emits an event, delivering it synchronously
 *
 * The event is delivered immediately: every matching listener is invoked before
 * this function returns. The payload is not copied (it only needs to stay valid
 * for the duration of the call). To instead queue an event for later delivery,
 * use {@link ecs_enqueue}.
 *
 * Because delivery is immediate, avoid emitting from within a running system if
 * a listener may perform component operations on the entities being iterated;
 * use {@link ecs_enqueue} there instead.
 *
 * @param ecs     The ECS context
 * @param event   The event to emit
 * @param payload Pointer to the payload. Must be non-NULL if the event was
 *                defined with a non-zero payload size, otherwise ignored.
 */
void ecs_emit(ecs_t* ecs, ecs_event_t event, const void* payload);

/**
 * @brief Emits an event tagged with a source id, delivering it synchronously
 *
 * Behaves like {@link ecs_emit} but associates the event with a source. The
 * event is delivered to listeners subscribed to that source via
 * {@link ecs_subscribe_to} as well as to unscoped listeners subscribed via
 * {@link ecs_subscribe}.
 *
 * @param ecs     The ECS context
 * @param event   The event to emit
 * @param source  The source id to tag the event with
 * @param payload Pointer to the payload. Must be non-NULL if the event was
 *                defined with a non-zero payload size, otherwise ignored.
 */
void ecs_emit_from(ecs_t* ecs,
                   ecs_event_t event,
                   ecs_id_t source,
                   const void* payload);

/**
 * @brief Enqueues an event for asynchronous (deferred) delivery
 *
 * The event is appended to the event queue and its payload copied, so the
 * caller need not keep the payload alive. Queued events are delivered to
 * subscribers only when {@link ecs_dispatch} is called. It is safe to call this
 * from within a running system or from a listener during dispatch.
 *
 * @param ecs     The ECS context
 * @param event   The event to enqueue
 * @param payload Pointer to the payload to copy. Must be non-NULL if the event
 *                was defined with a non-zero payload size, otherwise ignored.
 */
void ecs_enqueue(ecs_t* ecs, ecs_event_t event, const void* payload);

/**
 * @brief Enqueues an event tagged with a source id for deferred delivery
 *
 * Behaves like {@link ecs_enqueue} but associates the event with a source, with
 * the same source-matching semantics as {@link ecs_emit_from}.
 *
 * @param ecs     The ECS context
 * @param event   The event to enqueue
 * @param source  The source id to tag the event with
 * @param payload Pointer to the payload to copy. Must be non-NULL if the event
 *                was defined with a non-zero payload size, otherwise ignored.
 */
void ecs_enqueue_from(ecs_t* ecs,
                      ecs_event_t event,
                      ecs_id_t source,
                      const void* payload);

/**
 * @brief Delivers all queued events to their subscribers
 *
 * Drains the event queue, invoking each subscribed listener for every event
 * enqueued (via {@link ecs_enqueue}) in the order the events were enqueued.
 * Events enqueued by listeners during dispatch are delivered within the same
 * call. This function is never called automatically and is not re-entrant.
 *
 * @param ecs The ECS context
 */
void ecs_dispatch(ecs_t* ecs);

/**
 * @brief Returns the built-in event emitted when a component is added
 *
 * The event's source is the component (its id) and its payload carries the
 * entity, the component, and the args passed to ecs_add (the payload layout is
 * internal). Subscribe with {@link ecs_subscribe} to observe adds for every
 * component, or use {@link ecs_on_add} for a per-component callback.
 *
 * @param ecs The ECS context
 * @returns   The add event handle
 */
ecs_event_t ecs_get_add_event(ecs_t* ecs);

/**
 * @brief Returns the built-in event emitted when a component is removed
 *
 * The event's source is the component (its id) and its payload carries the
 * entity and the component (the payload layout is internal).
 *
 * @param ecs The ECS context
 * @returns   The remove event handle
 */
ecs_event_t ecs_get_remove_event(ecs_t* ecs);

/**
 * @brief Returns the built-in event emitted when a component's data is set
 *
 * The event's source is the component (its id) and its payload carries the
 * entity and the component (the payload layout is internal).
 *
 * @param ecs The ECS context
 * @returns   The set event handle
 */
ecs_event_t ecs_get_set_event(ecs_t* ecs);

/**
 * @brief Called when a component is added to an entity (via ecs_add)
 *
 * Registered with {@link ecs_on_add}. Delivered as a queued event, so it fires
 * on {@link ecs_dispatch} rather than synchronously during ecs_add.
 *
 * @param ecs    The ECS context
 * @param entity The entity the component was added to
 * @param comp   The component that was added
 * @param args   The args passed to ecs_add (copied; see ecs_define_component's
 *               args_size), or NULL
 * @param udata  The component's user data (see ecs_define_component)
 */
typedef void (*ecs_on_add_fn)(ecs_t* ecs,
                              ecs_entity_t entity,
                              ecs_comp_t comp,
                              const void* args,
                              void* udata);

/**
 * @brief Called when a component is removed from an entity (via ecs_remove or
 * ecs_destroy)
 *
 * Registered with {@link ecs_on_remove}. Delivered as a queued event, so it
 * fires on {@link ecs_dispatch}. Note that when triggered by ecs_destroy the
 * entity is already inactive by dispatch time, so the callback must not assume
 * the entity is still live.
 *
 * @param ecs    The ECS context
 * @param entity The entity the component was removed from
 * @param comp   The component that was removed
 * @param udata  The component's user data (see ecs_define_component)
 */
typedef void (*ecs_on_remove_fn)(ecs_t* ecs,
                                 ecs_entity_t entity,
                                 ecs_comp_t comp,
                                 void* udata);


/**
 * @brief Called when a component's data is set (via ecs_set)
 *
 * Registered with {@link ecs_on_set}. Delivered as a queued event, so it fires
 * on {@link ecs_dispatch} rather than synchronously during ecs_set.
 *
 * @param ecs    The ECS context
 * @param entity The entity whose component was set
 * @param comp   The component that was set
 * @param udata  The component's user data (see ecs_define_component)
 */
typedef void (*ecs_on_set_fn)(ecs_t* ecs,
                              ecs_entity_t entity,
                              ecs_comp_t comp,
                              void* udata);

/**
 * @brief Sets the callback invoked when the component is added to an entity
 *
 * The callback receives the added entity, the component, the args passed to
 * ecs_add, and the component's user data. It is delivered through a built-in
 * event and fires on {@link ecs_dispatch}, not synchronously during ecs_add.
 * Calling this again replaces the component's add callback.
 *
 * @param ecs  The ECS context
 * @param comp The component to watch
 * @param fn   The callback invoked when the component is added
 */
void ecs_on_add(ecs_t* ecs, ecs_comp_t comp, ecs_on_add_fn fn);

/**
 * @brief Sets the callback invoked when the component is removed from an entity
 *
 * The callback fires for both ecs_remove and ecs_destroy and is delivered
 * through a built-in event on {@link ecs_dispatch}, not synchronously. Calling
 * this again replaces the component's remove callback.
 *
 * @param ecs  The ECS context
 * @param comp The component to watch
 * @param fn   The callback invoked when the component is removed
 */
void ecs_on_remove(ecs_t* ecs, ecs_comp_t comp, ecs_on_remove_fn fn);

/**
 * @brief Sets the callback invoked when the component's data is set via ecs_set
 *
 * Delivered through a built-in event on {@link ecs_dispatch}, not
 * synchronously during ecs_set. Calling this again replaces the component's
 * set callback.
 *
 * @param ecs  The ECS context
 * @param comp The component to watch
 * @param fn   The callback invoked when the component is set
 */
void ecs_on_set(ecs_t* ecs, ecs_comp_t comp, ecs_on_set_fn fn);


/**
 * @brief Returns the built-in event emitted when an entity joins a system
 *
 * The event's source is the system (its id) and its payload carries the joining
 * entity and the system (the payload layout is internal). Subscribe with
 * {@link ecs_subscribe} to observe joins for every system, or use
 * {@link ecs_on_join} for a per-system callback that receives the entity.
 *
 * @param ecs The ECS context
 * @returns   The join event handle
 */
ecs_event_t ecs_get_join_event(ecs_t* ecs);

/**
 * @brief Returns the built-in event emitted when an entity leaves a system
 *
 * The event's source is the system (its id) and its payload carries the leaving
 * entity and the system (the payload layout is internal). An entity leaves when
 * it no longer matches the system or when it is destroyed.
 *
 * @param ecs The ECS context
 * @returns   The leave event handle
 */
ecs_event_t ecs_get_leave_event(ecs_t* ecs);

/**
 * @brief Called when an entity joins a system
 *
 * @param ecs    The ECS context
 * @param entity The entity that joined
 * @param udata  The user data passed to {@link ecs_on_join}
 */
typedef void (*ecs_on_join_fn)(ecs_t* ecs, ecs_entity_t entity, void* udata);

/**
 * @brief Called when an entity leaves a system
 *
 * @param ecs    The ECS context
 * @param entity The entity that left
 * @param udata  The user data passed to {@link ecs_on_leave}
 */
typedef void (*ecs_on_leave_fn)(ecs_t* ecs, ecs_entity_t entity, void* udata);

/**
 * @brief Sets the callback invoked when an entity joins a specific system
 *
 * The callback is delivered through the built-in join event (see
 * {@link ecs_get_join_event}), scoped to the given system, and receives the
 * joining entity along with the system's user data (see
 * {@link ecs_set_system_udata}). It fires on {@link ecs_dispatch}, not
 * synchronously. Calling this again replaces the system's join callback.
 *
 * @param ecs The ECS context
 * @param sys The system to watch
 * @param fn  The callback invoked for each entity that joins the system
 */
void ecs_on_join(ecs_t* ecs, ecs_system_t sys, ecs_on_join_fn fn);

/**
 * @brief Sets the callback invoked when an entity leaves a specific system
 *
 * The callback is delivered through the built-in leave event (see
 * {@link ecs_get_leave_event}), scoped to the given system, and receives the
 * leaving entity along with the system's user data (see
 * {@link ecs_set_system_udata}). It fires on {@link ecs_dispatch}, not
 * synchronously. An entity leaves when it no longer matches the system or when
 * it is destroyed. Calling this again replaces the system's leave callback.
 *
 * @param ecs The ECS context
 * @param sys The system to watch
 * @param fn  The callback invoked for each entity that leaves the system
 */
void ecs_on_leave(ecs_t* ecs, ecs_system_t sys, ecs_on_leave_fn fn);

#ifdef __cplusplus
}
#endif

#endif // PICO_ECS_H

#ifdef PICO_ECS_IMPLEMENTATION // Define once

#ifndef PICO_ECS_MAX_COMPONENTS
#define PICO_ECS_MAX_COMPONENTS 32
#endif

#ifndef PICO_ECS_MAX_SYSTEMS
#define PICO_ECS_MAX_SYSTEMS 16
#endif

#ifndef PICO_ECS_COMP_BLOCK_SIZE
#define PICO_ECS_COMP_BLOCK_SIZE 64
#endif

#ifndef PICO_ECS_MAX_EVENTS
#define PICO_ECS_MAX_EVENTS 32
#endif

#ifndef PICO_ECS_MAX_LISTENERS
#define PICO_ECS_MAX_LISTENERS 16
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

#ifndef PICO_ECS_MEMSET
    #include <string.h>
    #define PICO_ECS_MEMSET memset
#endif

#ifndef PICO_ECS_MEMCPY
    #include <string.h>
    #define PICO_ECS_MEMCPY memcpy
#endif

#include <stdalign.h>

/*=============================================================================
 *  Aliases>
 *============================================================================*/

#define ECS_ASSERT           PICO_ECS_ASSERT
#define ECS_MAX_COMPONENTS   PICO_ECS_MAX_COMPONENTS
#define ECS_MAX_SYSTEMS      PICO_ECS_MAX_SYSTEMS
#define ECS_COMP_BLOCK_SIZE  PICO_ECS_COMP_BLOCK_SIZE
#define ECS_MAX_EVENTS       PICO_ECS_MAX_EVENTS
#define ECS_MAX_LISTENERS    PICO_ECS_MAX_LISTENERS
#define ECS_MALLOC           PICO_ECS_MALLOC
#define ECS_REALLOC          PICO_ECS_REALLOC
#define ECS_FREE             PICO_ECS_FREE
#define ECS_MEMSET           PICO_ECS_MEMSET
#define ECS_MEMCPY           PICO_ECS_MEMCPY

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

typedef struct
{
    ecs_entity_t entity;
    ecs_system_t system;
} ecs_sys_event_t;

typedef struct
{
    ecs_entity_t entity;
    ecs_comp_t   comp;
    const void*  args;
} ecs_comp_event_t;


typedef struct ecs_arena_block_s
{
    uint8_t*              memory;
    size_t                size;
    size_t                offset;
    struct ecs_arena_block_s* next;
} ecs_arena_block_t;

typedef struct ecs_arena_s
{
    ecs_t* ecs;
    ecs_arena_block_t* first;
    ecs_arena_block_t* current;
    size_t block_size;
} ecs_arena_t;

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
    size_t  block_count;    // number of allocated blocks
    size_t  block_capacity; // capacity of the blocks[] pointer array
    size_t  comp_size;      // size of one component in bytes
    void**  blocks;         // array of pointers to fixed-size blocks
} ecs_comp_blocks_t;

typedef struct
{
    ecs_bitset_t comp_bits;
    bool         active;
    bool         ready;
} ecs_entity_data_t;

typedef struct
{
    ecs_on_add_fn on_add;
    ecs_on_remove_fn on_remove;
    ecs_on_set_fn on_set;
    size_t size;
    void* default_value;
    size_t args_size;
    void* udata;
} ecs_comp_data_t;

typedef struct
{
    bool             active;
    ecs_sparse_set_t entity_ids;
    ecs_mask_t       mask;
    ecs_system_fn    system_cb;
    ecs_on_join_fn   on_join;  // set via ecs_on_join, delivered as an event
    ecs_on_leave_fn  on_leave; // set via ecs_on_leave, delivered as an event
    ecs_bitset_t     require_bits;
    ecs_bitset_t     exclude_bits;
    void*            udata;
} ecs_sys_data_t;

typedef enum
{
    ECS_CMD_ADD,
    ECS_CMD_REMOVE,
    ECS_CMD_SET,
    ECS_CMD_DESTROY,
} ecs_cmd_type_t;

typedef struct
{
    ecs_cmd_type_t type;
    ecs_entity_t   entity;
    ecs_comp_t     comp;
    void*          data;
    void*          args;
} ecs_cmd_t;

typedef struct
{
    ecs_cmd_t* data;
    size_t     size;
    size_t     capacity;
} ecs_cmd_array_t;

typedef struct
{
    ecs_listener_fn fn;
    void*           udata;
    ecs_id_t        source; // only matching-source events are delivered;
                            // ECS_INVALID_ID receives events from any source
    bool            active;
} ecs_listener_t;

typedef struct
{
    bool           active;       // true once defined
    size_t         payload_size;
    ecs_listener_t listeners[ECS_MAX_LISTENERS];
    size_t         listener_count;
} ecs_event_data_t;

typedef struct
{
    ecs_id_t event_id;
    ecs_id_t source;  // ECS_INVALID_ID if emitted without a source
    void*    payload; // arena-allocated copy, NULL if payload_size is 0
} ecs_event_msg_t;

typedef struct
{
    ecs_event_msg_t* data;
    size_t           size;
    size_t           capacity;
} ecs_event_queue_t;

struct ecs_s
{
    ecs_id_array_t     entity_pool;
    ecs_entity_data_t* entities;
    size_t             entity_capacity;
    size_t             next_entity_id;
    ecs_comp_data_t    comps[ECS_MAX_COMPONENTS];
    ecs_comp_blocks_t  comp_blocks[ECS_MAX_COMPONENTS];
    size_t             comp_count;
    ecs_sys_data_t     systems[ECS_MAX_SYSTEMS];
    size_t             system_count;
    bool               system_active;
    ecs_cmd_array_t    cmd_queue;
    ecs_arena_t        arena;
    ecs_event_data_t   events[ECS_MAX_EVENTS];
    size_t             event_count;
    ecs_event_queue_t  event_queue;
    ecs_arena_t        event_arena;
    ecs_event_t        join_event;   // built-in: entity joined a system
    ecs_event_t        leave_event;  // built-in: entity left a system
    ecs_event_t        add_event;    // built-in: component added to an entity
    ecs_event_t        remove_event; // built-in: component removed
    ecs_event_t        set_event;    // built-in: component data set
    bool               dispatching;
    void*              mem_ctx;
};

/*=============================================================================
 * Handle constructors
 *============================================================================*/
static inline ecs_entity_t ecs_make_entity(ecs_id_t id);
static inline ecs_comp_t ecs_make_comp(ecs_id_t id);
static inline ecs_system_t ecs_make_system(ecs_id_t id);
static inline ecs_event_t ecs_make_event(ecs_id_t id);

/*=============================================================================
 * Built-in lifecycle event forwarding listeners
 *============================================================================*/
static void ecs_enqueue_impl(ecs_t* ecs, ecs_event_t event, ecs_id_t source, const void* payload);
static inline bool ecs_event_has_user_listener(ecs_t* ecs, ecs_event_t event);

static void ecs_forward_on_add(ecs_t* ecs, ecs_event_t event,
                               const void* payload, size_t payload_size, void* udata);
static void ecs_forward_on_remove(ecs_t* ecs, ecs_event_t event,
                                  const void* payload, size_t payload_size, void* udata);
static void ecs_forward_on_set(ecs_t* ecs, ecs_event_t event,
                               const void* payload, size_t payload_size, void* udata);
static void ecs_forward_on_join(ecs_t* ecs, ecs_event_t event,
                                const void* payload, size_t payload_size, void* udata);
static void ecs_forward_on_leave(ecs_t* ecs, ecs_event_t event,
                                 const void* payload, size_t payload_size, void* udata);

/*=============================================================================
 * Realloc wrapper
 *============================================================================*/
static void* ecs_realloc_zero(ecs_t* ecs, void* ptr, size_t old_size, size_t new_size);

/*=============================================================================
 * Tests if entity is active (created)
 *============================================================================*/
static inline bool ecs_is_active(ecs_t* ecs, ecs_id_t entity_id);

/*=============================================================================
 * Command queue functions
 *============================================================================*/
static void  ecs_cmd_array_init(ecs_t* ecs, ecs_cmd_array_t* queue, size_t capacity);
static void  ecs_cmd_array_free(ecs_t* ecs, ecs_cmd_array_t* queue);
static ecs_cmd_t* ecs_cmd_array_push(ecs_t* ecs, ecs_cmd_array_t* queue);
static void  ecs_cmd_flush_queue(ecs_t* ecs);

/*=============================================================================
 * Event queue functions
 *============================================================================*/
static void ecs_event_queue_init(ecs_t* ecs, ecs_event_queue_t* queue, size_t capacity);
static void ecs_event_queue_free(ecs_t* ecs, ecs_event_queue_t* queue);
static ecs_event_msg_t* ecs_event_queue_push(ecs_t* ecs, ecs_event_queue_t* queue);

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
 * Arena functions
 *============================================================================*/
static ecs_arena_block_t* ecs_arena_block_create(ecs_t* ecs, size_t size);
static bool ecs_arena_init(ecs_t* ecs, ecs_arena_t* arena, size_t initial_block_size);
static bool ecs_arena_grow(ecs_t* ecs, ecs_arena_t* arena, size_t min_size);
static uintptr_t ecs_arena_align_forward(uintptr_t ptr, size_t align);
static void* ecs_arena_alloc_align(ecs_t* ecs, ecs_arena_t* arena, size_t size, size_t align);
static void* ecs_arena_alloc(ecs_t* ecs, ecs_arena_t* arena, size_t size);
static void ecs_arena_reset(ecs_t* ecs, ecs_arena_t* arena);
static void ecs_arena_destroy(ecs_t* ecs, ecs_arena_t* arena);

/*=============================================================================
 * Sparse set functions
 *============================================================================*/
static void ecs_sparse_set_init(ecs_t* ecs, ecs_sparse_set_t* set, size_t capacity);
static void ecs_sparse_set_free(ecs_t* ecs, ecs_sparse_set_t* set);
static bool ecs_sparse_set_add(ecs_t* ecs, ecs_sparse_set_t* set, ecs_id_t id);
static inline bool ecs_sparse_set_find(ecs_sparse_set_t* set, ecs_id_t id, size_t* found);
static inline bool ecs_sparse_set_remove(ecs_sparse_set_t* set, ecs_id_t id);

/*=============================================================================
 * System entity add/remove functions
 *============================================================================*/

static bool ecs_entity_system_test(ecs_bitset_t require_bits,
                                   ecs_bitset_t exclude_bits,
                                   ecs_bitset_t entity_bits);

static void ecs_sync_add_remove(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id);
static void ecs_sync_destroy(ecs_t* ecs, ecs_id_t entity_id);

/*=============================================================================
 * ID array functions
 *============================================================================*/
static void   ecs_id_array_init(ecs_t* ecs, ecs_id_array_t* pool, size_t capacity);
static void   ecs_id_array_free(ecs_t* ecs, ecs_id_array_t* pool);
static inline void  ecs_id_array_push(ecs_t* ecs, ecs_id_array_t* pool, ecs_id_t id);
static inline ecs_id_t ecs_id_array_pop(ecs_id_array_t* pool);
static inline size_t ecs_id_array_size(ecs_id_array_t* pool);

/*=============================================================================
 * Component array functions
 *============================================================================*/
static void ecs_comp_blocks_init(ecs_t* ecs, ecs_comp_blocks_t* array, size_t size, size_t capacity);
static void ecs_comp_blocks_free(ecs_t* ecs, ecs_comp_blocks_t* array);
static void ecs_comp_blocks_resize(ecs_t* ecs, ecs_comp_blocks_t* array, ecs_id_t id);

/*=============================================================================
 * Validation functions
 *============================================================================*/
#ifndef NDEBUG
static bool ecs_is_not_null(void* ptr);
static bool ecs_is_valid_component_id(ecs_id_t id);
static bool ecs_is_valid_system_id(ecs_id_t id);
static bool ecs_is_valid_event_id(ecs_id_t id);
static bool ecs_is_valid_id(ecs_id_t id);
static bool ecs_is_valid_capacity(size_t capacity, size_t elem_size);
static bool ecs_is_entity_ready(ecs_t* ecs, ecs_id_t entity_id);
static bool ecs_is_component_ready(ecs_t* ecs, ecs_id_t comp_id);
static bool ecs_is_system_ready(ecs_t* ecs, ecs_id_t sys_id);
static bool ecs_is_event_ready(ecs_t* ecs, ecs_id_t event_id);
#endif // NDEBUG

/*=============================================================================
 * Public API implementation
 *============================================================================*/

ecs_t* ecs_new(size_t entity_capacity, void* mem_ctx)
{
    ECS_ASSERT(entity_capacity > 0);
    ECS_ASSERT(!ecs_is_valid_id(ECS_INVALID_ID) && "ecs_id_t is signed");

    ecs_t* ecs = (ecs_t*)ECS_MALLOC(sizeof(ecs_t), mem_ctx);

    // Out of memory
    if (NULL == ecs)
        return NULL;

    ECS_MEMSET(ecs, 0, sizeof(ecs_t));

    ecs->entity_capacity = (entity_capacity > 0) ? entity_capacity : 32;
    ecs->next_entity_id  = 0;
    ecs->system_active   = false;
    ecs->mem_ctx         = mem_ctx;

    // Initialize entity pool and queues
    ecs_id_array_init(ecs, &ecs->entity_pool, entity_capacity);

    // Initialize deferred command queue
    ecs_cmd_array_init(ecs, &ecs->cmd_queue, entity_capacity);

    // Initialize event queue
    ecs_event_queue_init(ecs, &ecs->event_queue, entity_capacity);

    // Allocate entity array
    ECS_ASSERT(ecs_is_valid_capacity(ecs->entity_capacity, sizeof(ecs_entity_data_t)));
    ecs->entities = (ecs_entity_data_t*)ECS_MALLOC(ecs->entity_capacity * sizeof(ecs_entity_data_t),
                                                   ecs->mem_ctx);

    // Zero entity array
    ECS_MEMSET(ecs->entities, 0, ecs->entity_capacity * sizeof(ecs_entity_data_t));

    ecs_arena_init(ecs, &ecs->arena, 512);
    ecs_arena_init(ecs, &ecs->event_arena, 512);

    // Reserve the built-in lifecycle events and subscribe a single forwarding
    // listener for each. Each listener looks up the system or component named in
    // the event payload and dispatches to its callback, so neither systems nor
    // components consume a listener slot per registration.
    ecs->join_event   = ecs_define_event(ecs, sizeof(ecs_sys_event_t));
    ecs->leave_event  = ecs_define_event(ecs, sizeof(ecs_sys_event_t));
    ecs->add_event    = ecs_define_event(ecs, sizeof(ecs_comp_event_t));
    ecs->remove_event = ecs_define_event(ecs, sizeof(ecs_comp_event_t));
    ecs->set_event    = ecs_define_event(ecs, sizeof(ecs_comp_event_t));

    ecs_subscribe(ecs, ecs->join_event,   ecs_forward_on_join,   NULL);
    ecs_subscribe(ecs, ecs->leave_event,  ecs_forward_on_leave,  NULL);
    ecs_subscribe(ecs, ecs->add_event,    ecs_forward_on_add,    NULL);
    ecs_subscribe(ecs, ecs->remove_event, ecs_forward_on_remove, NULL);
    ecs_subscribe(ecs, ecs->set_event,    ecs_forward_on_set,    NULL);

    return ecs;
}

void ecs_free(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    ecs_id_array_free(ecs, &ecs->entity_pool);
    ecs_cmd_array_free(ecs, &ecs->cmd_queue);
    ecs_event_queue_free(ecs, &ecs->event_queue);
    ecs_arena_destroy(ecs, &ecs->arena);
    ecs_arena_destroy(ecs, &ecs->event_arena);

    for (ecs_id_t comp_id = 0; comp_id < ecs->comp_count; comp_id++)
    {
        ecs_comp_blocks_t* comp_blocks = &ecs->comp_blocks[comp_id];
        ecs_comp_blocks_free(ecs, comp_blocks);
    }

    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++)
    {
        ecs_sys_data_t* sys = &ecs->systems[sys_id];
        ecs_sparse_set_free(ecs, &sys->entity_ids);
    }

    for (ecs_id_t comp_id = 0; comp_id < ecs->comp_count; comp_id++)
    {
        ecs_comp_data_t* comp_data = &ecs->comps[comp_id];

        if (comp_data->default_value)
        {
            ECS_FREE(comp_data->default_value, ecs->mem_ctx);
        }
    }

    ECS_FREE(ecs->entities, ecs->mem_ctx);
    ECS_FREE(ecs, ecs->mem_ctx);
}

void ecs_reset(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    ecs->entity_pool.size = 0;

    ECS_MEMSET(ecs->entities, 0, ecs->entity_capacity * sizeof(ecs_entity_data_t));

    ecs->next_entity_id = 0;

    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++)
    {
        ecs->systems[sys_id].entity_ids.size = 0;
    }

    // Discard any pending events (subscriptions are preserved)
    ecs->event_queue.size = 0;
    ecs_arena_reset(ecs, &ecs->event_arena);
}

ecs_comp_t ecs_define_component(ecs_t* ecs,
                                size_t size,
                                const ecs_comp_desc_t* desc)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs->comp_count < ECS_MAX_COMPONENTS);
    ECS_ASSERT(size > 0);

    ecs_comp_t comp = ecs_make_comp(ecs->comp_count);

    ecs_comp_blocks_t* comp_blocks = &ecs->comp_blocks[comp.id];
    ecs_comp_blocks_init(ecs, comp_blocks, size, ecs->entity_capacity);

    ecs_comp_data_t* comp_data = &ecs->comps[comp.id];

    ECS_MEMSET(comp_data, 0, sizeof(ecs_comp_data_t));
    comp_data->size = size;

    if (desc)
    {
        comp_data->args_size = desc->args_size;
        comp_data->udata = desc->udata;

        if (desc->default_value)
        {
            comp_data->default_value = ECS_MALLOC(size, ecs->mem_ctx);
            ECS_MEMCPY(comp_data->default_value, desc->default_value, size);
        }
    }

    ecs->comp_count++;

    return comp;
}

// Forwarding listeners for the built-in component events. Each looks up the
// component named in the payload and, if it has a callback, invokes it with the
// component's user data. A single listener handles every component.
static void ecs_forward_on_add(ecs_t* ecs, ecs_event_t event,
                               const void* payload, size_t payload_size, void* udata)
{
    (void)event;
    (void)payload_size;
    (void)udata;

    const ecs_comp_event_t* msg = (const ecs_comp_event_t*)payload;
    ecs_comp_data_t* comp_data = &ecs->comps[msg->comp.id];

    if (comp_data->on_add)
        comp_data->on_add(ecs, msg->entity, msg->comp, msg->args, comp_data->udata);
}

static void ecs_forward_on_remove(ecs_t* ecs, ecs_event_t event,
                                  const void* payload, size_t payload_size, void* udata)
{
    (void)event;
    (void)payload_size;
    (void)udata;

    const ecs_comp_event_t* msg = (const ecs_comp_event_t*)payload;
    ecs_comp_data_t* comp_data = &ecs->comps[msg->comp.id];

    if (comp_data->on_remove)
        comp_data->on_remove(ecs, msg->entity, msg->comp, comp_data->udata);
}

static void ecs_forward_on_set(ecs_t* ecs, ecs_event_t event,
                               const void* payload, size_t payload_size, void* udata)
{
    (void)event;
    (void)payload_size;
    (void)udata;

    const ecs_comp_event_t* msg = (const ecs_comp_event_t*)payload;
    ecs_comp_data_t* comp_data = &ecs->comps[msg->comp.id];

    if (comp_data->on_set)
        comp_data->on_set(ecs, msg->entity, msg->comp, comp_data->udata);
}

void ecs_on_add(ecs_t* ecs, ecs_comp_t comp, ecs_on_add_fn fn)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_component_id(comp.id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp.id));
    ECS_ASSERT(NULL != fn);

    ecs->comps[comp.id].on_add = fn;
}

void ecs_on_remove(ecs_t* ecs, ecs_comp_t comp, ecs_on_remove_fn fn)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_component_id(comp.id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp.id));
    ECS_ASSERT(NULL != fn);

    ecs->comps[comp.id].on_remove = fn;
}

void ecs_on_set(ecs_t* ecs, ecs_comp_t comp, ecs_on_set_fn fn)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_component_id(comp.id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp.id));
    ECS_ASSERT(NULL != fn);

    ecs->comps[comp.id].on_set = fn;
}

ecs_system_t ecs_define_system(ecs_t* ecs,
                               ecs_system_fn system_cb,
                               const ecs_sys_desc_t* desc)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs->system_count < ECS_MAX_SYSTEMS);
    ECS_ASSERT(NULL != system_cb);

    ecs_system_t sys = ecs_make_system(ecs->system_count);
    ecs_sys_data_t* sys_data = &ecs->systems[sys.id];

    ECS_MEMSET(sys_data, 0, sizeof(ecs_sys_data_t));

    ecs_sparse_set_init(ecs, &sys_data->entity_ids, ecs->entity_capacity);

    sys_data->system_cb = system_cb;
    sys_data->active = true;

    if (desc)
    {
        sys_data->mask = desc->mask;
        sys_data->udata = desc->udata;
    }

    ecs->system_count++;

    return sys;
}

void ecs_require(ecs_t* ecs, ecs_system_t sys, ecs_comp_t comp)
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

void ecs_exclude(ecs_t* ecs, ecs_system_t sys, ecs_comp_t comp)
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

void ecs_set_system_callback(ecs_t* ecs,
                             ecs_system_t sys,
                             ecs_system_fn system_cb)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys.id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys.id));
    ECS_ASSERT(NULL != system_cb);

    ecs_sys_data_t* sys_data = &ecs->systems[sys.id];
    sys_data->system_cb = system_cb;
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

ecs_entity_t* ecs_get_entity_array(ecs_t* ecs, ecs_system_t sys)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys.id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys.id));

    return ecs->systems[sys.id].entity_ids.dense;
}

size_t ecs_get_entity_count(ecs_t* ecs, ecs_system_t sys)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys.id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys.id));

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
        if (entity_id >= ecs->entity_capacity)
        {
            size_t old_capacity = ecs->entity_capacity;
            size_t new_capacity = 2 * old_capacity;

            ECS_ASSERT(ecs_is_valid_capacity(new_capacity, sizeof(ecs_entity_data_t)));
            ecs->entities = (ecs_entity_data_t*)ecs_realloc_zero(ecs, ecs->entities,
                                                                 old_capacity * sizeof(ecs_entity_data_t),
                                                                 new_capacity * sizeof(ecs_entity_data_t));

            ecs->entity_capacity = new_capacity;
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

void ecs_set(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp, void* data)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_id(entity.id));
    ECS_ASSERT(ecs_is_valid_component_id(comp.id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp.id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity.id));

    if (!ecs_has(ecs, entity, comp))
    {
        ecs_add(ecs, entity, comp, NULL);
    }

    ecs_comp_data_t* comp_data = &ecs->comps[comp.id];

    if (ecs->system_active)
    {
        ecs_cmd_t* cmd = ecs_cmd_array_push(ecs, &ecs->cmd_queue);
        cmd->type   = ECS_CMD_SET;
        cmd->entity = entity;
        cmd->comp   = comp;
        cmd->data   = ecs_arena_alloc(ecs, &ecs->arena, comp_data->size);
        ECS_MEMCPY(cmd->data, data, comp_data->size);
        return;
    }

    void* comp_ptr = ecs_get(ecs, entity, comp);
    ECS_MEMCPY(comp_ptr, data, comp_data->size);

    // Queue the on_set callback (delivered on ecs_dispatch)
    if (comp_data->on_set || ecs_event_has_user_listener(ecs, ecs->set_event))
    {
        ecs_comp_event_t msg = { entity, comp, NULL };
        ecs_enqueue_impl(ecs, ecs->set_event, comp.id, &msg);
    }
}

void ecs_destroy(ecs_t* ecs, ecs_entity_t entity)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_id(entity.id));
    ECS_ASSERT(ecs_is_active(ecs, entity.id));

    if (!ecs_is_active(ecs, entity.id))
        return;

    ecs_entity_data_t* entity_data = &ecs->entities[entity.id];
    ecs_bitset_t comp_bits = entity_data->comp_bits;

    if (ecs->system_active)
    {
        ecs_cmd_t* cmd = ecs_cmd_array_push(ecs, &ecs->cmd_queue);
        cmd->type   = ECS_CMD_DESTROY;
        cmd->entity = entity;
        ecs->entities[entity.id].ready = false;
        return;
    }

    for (ecs_id_t comp_id = 0; comp_id < ecs->comp_count; comp_id++)
    {
        if (ecs_bitset_test(&comp_bits, comp_id))
        {
            ecs_comp_data_t* comp_data = &ecs->comps[comp_id];

            // Queue the on_remove callback (delivered on ecs_dispatch). The
            // entity is deactivated below, so the callback runs after the entity
            // is gone.
            if (comp_data->on_remove || ecs_event_has_user_listener(ecs, ecs->remove_event))
            {
                ecs_comp_t comp = ecs_make_comp(comp_id);
                ecs_comp_event_t msg = { entity, comp, NULL };
                ecs_enqueue_impl(ecs, ecs->remove_event, comp_id, &msg);
            }
        }
    }

    ecs_sync_destroy(ecs, entity.id);

    ecs_id_array_t* pool = &ecs->entity_pool;
    ecs_id_array_push(ecs, pool, entity.id);

    ECS_MEMSET(&entity_data->comp_bits, 0, sizeof(ecs_bitset_t));
    entity_data->active = false;
    entity_data->ready  = false;
}

bool ecs_has(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_id(entity.id));
    ECS_ASSERT(ecs_is_valid_component_id(comp.id));

    // Load entity data
    ecs_entity_data_t* entity_data = &ecs->entities[entity.id];

    if (!entity_data->ready)
        return false;

    // Return true if the component belongs to the entity
    return ecs_bitset_test(&entity_data->comp_bits, comp.id);
}

void* ecs_get(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_id(entity.id));
    ECS_ASSERT(ecs_is_valid_component_id(comp.id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp.id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity.id));

    // Map entity ID to block and slot within that block.
    // Blocks are never reallocated, so returned pointers remain stable.
    ecs_comp_blocks_t* comp_blocks = &ecs->comp_blocks[comp.id];

    size_t block = entity.id / ECS_COMP_BLOCK_SIZE;
    size_t slot  = entity.id % ECS_COMP_BLOCK_SIZE;

    return (char*)comp_blocks->blocks[block] + (comp_blocks->comp_size * slot);
}

void ecs_add(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp, void* args)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_id(entity.id));
    ECS_ASSERT(ecs_is_valid_component_id(comp.id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity.id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp.id));

    if (ecs_has(ecs, entity, comp))
        return;

    // Load entity data
    ecs_entity_data_t* entity_data = &ecs->entities[entity.id];

    // Set entity component bit that determines which systems this entity
    // belongs to
    ecs_bitset_flip(&entity_data->comp_bits, comp.id, true);

    // Load component
    ecs_comp_blocks_t* comp_blocks = &ecs->comp_blocks[comp.id];

    // Grow the component array now (not deferred) so that ecs_get can safely
    // index into it immediately, since ecs_has already reports the component
    // as present as soon as the bit above is flipped
    ecs_comp_blocks_resize(ecs, comp_blocks, entity.id);

    ecs_comp_data_t* comp_data = &ecs->comps[comp.id];

    if (ecs->system_active)
    {
        ecs_cmd_t* cmd = ecs_cmd_array_push(ecs, &ecs->cmd_queue);
        cmd->type      = ECS_CMD_ADD;
        cmd->entity    = entity;
        cmd->comp      = comp;

        // Copy the constructor args into the command arena so the caller is
        // not required to keep them alive until the queue is flushed. The copy
        // is handed back to ecs_add (and thus the on_add constructor) when the
        // queue is flushed at the end of the system run.
        if (args && comp_data->args_size > 0)
        {
            cmd->args = ecs_arena_alloc(ecs, &ecs->arena, comp_data->args_size);
            ECS_MEMCPY(cmd->args, args, comp_data->args_size);
        }

        return;
    }

    // Get pointer to component
    void* comp_ptr = ecs_get(ecs, entity, comp);

    // Set default value
    if (comp_data->default_value)
        ECS_MEMCPY(comp_ptr, comp_data->default_value, comp_data->size);
    else
        ECS_MEMSET(comp_ptr, 0, comp_blocks->comp_size);

    // Queue the on_add callback (delivered on ecs_dispatch). The args are
    // copied into the event arena so the caller need not keep them alive until
    // dispatch; this requires the component to define a non-zero args_size.
    if (comp_data->on_add || ecs_event_has_user_listener(ecs, ecs->add_event))
    {
        const void* args_copy = NULL;

        if (args && comp_data->args_size > 0)
        {
            void* copy = ecs_arena_alloc(ecs, &ecs->event_arena, comp_data->args_size);
            ECS_MEMCPY(copy, args, comp_data->args_size);
            args_copy = copy;
        }

        ecs_comp_event_t msg = { entity, comp, args_copy };
        ecs_enqueue_impl(ecs, ecs->add_event, comp.id, &msg);
    }

    // Add/remove entity to/from systems based on matching criteria
    ecs_sync_add_remove(ecs, entity.id, comp.id);
}

void ecs_remove(ecs_t* ecs, ecs_entity_t entity, ecs_comp_t comp)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_id(entity.id));
    ECS_ASSERT(ecs_is_valid_component_id(comp.id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp.id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity.id));

    if (!ecs_has(ecs, entity, comp))
        return;

    // Load entity data
    ecs_entity_data_t* entity_data = &ecs->entities[entity.id];

    // Set entity component bit that determines which systems this entity
    // belongs to
    ecs_bitset_flip(&entity_data->comp_bits, comp.id, false);

    if (ecs->system_active)
    {
        ecs_cmd_t* cmd = ecs_cmd_array_push(ecs, &ecs->cmd_queue);
        cmd->type   = ECS_CMD_REMOVE;
        cmd->entity = entity;
        cmd->comp   = comp;
        return;
    }

    // Queue the on_remove callback (delivered on ecs_dispatch)
    ecs_comp_data_t* comp_data = &ecs->comps[comp.id];

    if (comp_data->on_remove || ecs_event_has_user_listener(ecs, ecs->remove_event))
    {
        ecs_comp_event_t msg = { entity, comp, NULL };
        ecs_enqueue_impl(ecs, ecs->remove_event, comp.id, &msg);
    }

    // Add/remove entity to/from systems based on matching criteria
    ecs_sync_add_remove(ecs, entity.id, comp.id);
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

    ecs->system_active = true;

    ecs_ret_t code = sys_data->system_cb(ecs,
                     sys_data->entity_ids.dense,
                     sys_data->entity_ids.size,
                     sys_data->udata);

    ecs->system_active = false;

    ecs_cmd_flush_queue(ecs);
    ecs_arena_reset(ecs, &ecs->arena);

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

ecs_event_t ecs_define_event(ecs_t* ecs, size_t payload_size)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs->event_count < ECS_MAX_EVENTS);

    ecs_event_t event = ecs_make_event(ecs->event_count);
    ecs_event_data_t* event_data = &ecs->events[event.id];

    ECS_MEMSET(event_data, 0, sizeof(ecs_event_data_t));
    event_data->active       = true;
    event_data->payload_size = payload_size;

    ecs->event_count++;

    return event;
}

static ecs_id_t ecs_subscribe_impl(ecs_t* ecs,
                                   ecs_event_t event,
                                   ecs_id_t source,
                                   ecs_listener_fn fn,
                                   void* udata)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_event_id(event.id));
    ECS_ASSERT(ecs_is_event_ready(ecs, event.id));
    ECS_ASSERT(NULL != fn);

    ecs_event_data_t* event_data = &ecs->events[event.id];

    // Reuse a previously unsubscribed slot if one exists so handles stay stable
    for (ecs_id_t sub = 0; sub < event_data->listener_count; sub++)
    {
        if (!event_data->listeners[sub].active)
        {
            event_data->listeners[sub].fn     = fn;
            event_data->listeners[sub].udata  = udata;
            event_data->listeners[sub].source = source;
            event_data->listeners[sub].active = true;
            return sub;
        }
    }

    ECS_ASSERT(event_data->listener_count < ECS_MAX_LISTENERS);

    ecs_id_t sub = event_data->listener_count++;
    event_data->listeners[sub].fn     = fn;
    event_data->listeners[sub].udata  = udata;
    event_data->listeners[sub].source = source;
    event_data->listeners[sub].active = true;

    return sub;
}

ecs_id_t ecs_subscribe(ecs_t* ecs,
                       ecs_event_t event,
                       ecs_listener_fn fn,
                       void* udata)
{
    // ECS_INVALID_ID as the source means "receive events from any source"
    return ecs_subscribe_impl(ecs, event, ECS_INVALID_ID, fn, udata);
}

ecs_id_t ecs_subscribe_to(ecs_t* ecs,
                          ecs_event_t event,
                          ecs_id_t source,
                          ecs_listener_fn fn,
                          void* udata)
{
    ECS_ASSERT(source != ECS_INVALID_ID && "use ecs_subscribe to match any source");
    return ecs_subscribe_impl(ecs, event, source, fn, udata);
}

void ecs_unsubscribe(ecs_t* ecs, ecs_event_t event, ecs_id_t sub)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_event_id(event.id));
    ECS_ASSERT(ecs_is_event_ready(ecs, event.id));

    ecs_event_data_t* event_data = &ecs->events[event.id];
    ECS_ASSERT(sub < event_data->listener_count);

    event_data->listeners[sub].active = false;
}

// Immediately invokes every matching listener for an event. Used by the
// synchronous ecs_emit and to drain queued (enqueued) events in ecs_dispatch.
static void ecs_deliver(ecs_t* ecs,
                        ecs_event_t event,
                        ecs_id_t source,
                        const void* payload)
{
    ecs_event_data_t* event_data = &ecs->events[event.id];
    size_t payload_size = event_data->payload_size;

    for (size_t j = 0; j < event_data->listener_count; j++)
    {
        // Copy by value so unsubscribing within a listener is well-defined
        ecs_listener_t listener = event_data->listeners[j];

        if (!listener.active)
            continue;

        // Unscoped listeners (source == ECS_INVALID_ID) receive every event;
        // scoped listeners only receive matching-source events
        if (listener.source != ECS_INVALID_ID && listener.source != source)
            continue;

        listener.fn(ecs, event, payload, payload_size, listener.udata);
    }
}

void ecs_emit(ecs_t* ecs, ecs_event_t event, const void* payload)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_event_id(event.id));
    ECS_ASSERT(ecs_is_event_ready(ecs, event.id));
    ECS_ASSERT(ecs->events[event.id].payload_size == 0 || payload != NULL);

    ecs_deliver(ecs, event, ECS_INVALID_ID, payload);
}

void ecs_emit_from(ecs_t* ecs,
                   ecs_event_t event,
                   ecs_id_t source,
                   const void* payload)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_event_id(event.id));
    ECS_ASSERT(ecs_is_event_ready(ecs, event.id));
    ECS_ASSERT(ecs->events[event.id].payload_size == 0 || payload != NULL);

    ecs_deliver(ecs, event, source, payload);
}

// Appends an event to the queue, copying its payload into the event arena so
// the caller need not keep it alive until ecs_dispatch.
static void ecs_enqueue_impl(ecs_t* ecs,
                             ecs_event_t event,
                             ecs_id_t source,
                             const void* payload)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_event_id(event.id));
    ECS_ASSERT(ecs_is_event_ready(ecs, event.id));

    ecs_event_data_t* event_data = &ecs->events[event.id];

    ecs_event_msg_t* msg = ecs_event_queue_push(ecs, &ecs->event_queue);
    msg->event_id = event.id;
    msg->source   = source;

    if (event_data->payload_size > 0)
    {
        ECS_ASSERT(payload != NULL);
        msg->payload = ecs_arena_alloc(ecs, &ecs->event_arena, event_data->payload_size);
        ECS_MEMCPY(msg->payload, payload, event_data->payload_size);
    }
    else
    {
        msg->payload = NULL;
    }
}

void ecs_enqueue(ecs_t* ecs, ecs_event_t event, const void* payload)
{
    ecs_enqueue_impl(ecs, event, ECS_INVALID_ID, payload);
}

void ecs_enqueue_from(ecs_t* ecs,
                      ecs_event_t event,
                      ecs_id_t source,
                      const void* payload)
{
    ecs_enqueue_impl(ecs, event, source, payload);
}

void ecs_dispatch(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(!ecs->dispatching && "ecs_dispatch is not re-entrant");

    ecs->dispatching = true;

    ecs_event_queue_t* queue = &ecs->event_queue;

    // Drain the queue. Listeners may enqueue new events, which are appended and
    // delivered within this same pass. The queue array may be reallocated by
    // those emissions, so values are read by index rather than cached pointers.
    for (size_t i = 0; i < queue->size; i++)
    {
        ecs_event_t event = ecs_make_event(queue->data[i].event_id);
        ecs_deliver(ecs, event, queue->data[i].source, queue->data[i].payload);
    }

    queue->size = 0;
    ecs_arena_reset(ecs, &ecs->event_arena);

    ecs->dispatching = false;
}

ecs_event_t ecs_get_join_event(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    return ecs->join_event;
}

ecs_event_t ecs_get_leave_event(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    return ecs->leave_event;
}

ecs_event_t ecs_get_add_event(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    return ecs->add_event;
}

ecs_event_t ecs_get_remove_event(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    return ecs->remove_event;
}

ecs_event_t ecs_get_set_event(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    return ecs->set_event;
}

// Forwarding listeners for the built-in system events. Each looks up the system
// named in the payload and, if it has a callback, invokes it with the entity and
// the system's user data. A single listener handles every system.
static void ecs_forward_on_join(ecs_t* ecs,
                                ecs_event_t event,
                                const void* payload,
                                size_t payload_size,
                                void* udata)
{
    (void)event;
    (void)payload_size;
    (void)udata;

    const ecs_sys_event_t* msg = (const ecs_sys_event_t*)payload;
    ecs_sys_data_t* sys_data = &ecs->systems[msg->system.id];

    if (sys_data->on_join)
        sys_data->on_join(ecs, msg->entity, sys_data->udata);
}

static void ecs_forward_on_leave(ecs_t* ecs,
                                 ecs_event_t event,
                                 const void* payload,
                                 size_t payload_size,
                                 void* udata)
{
    (void)event;
    (void)payload_size;
    (void)udata;

    const ecs_sys_event_t* msg = (const ecs_sys_event_t*)payload;
    ecs_sys_data_t* sys_data = &ecs->systems[msg->system.id];

    if (sys_data->on_leave)
        sys_data->on_leave(ecs, msg->entity, sys_data->udata);
}

void ecs_on_join(ecs_t* ecs, ecs_system_t sys, ecs_on_join_fn fn)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys.id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys.id));
    ECS_ASSERT(NULL != fn);

    ecs->systems[sys.id].on_join = fn;
}

void ecs_on_leave(ecs_t* ecs, ecs_system_t sys, ecs_on_leave_fn fn)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys.id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys.id));
    ECS_ASSERT(NULL != fn);

    ecs->systems[sys.id].on_leave = fn;
}

/*=============================================================================
 * Handle constructors
 *============================================================================*/
static inline ecs_entity_t ecs_make_entity(ecs_id_t id)
{
    return (ecs_entity_t){ id };
}

static inline ecs_comp_t ecs_make_comp(ecs_id_t id)
{
    return (ecs_comp_t){ id };
}

static inline ecs_system_t ecs_make_system(ecs_id_t id)
{
    return (ecs_system_t){ id };
}

static inline ecs_event_t ecs_make_event(ecs_id_t id)
{
    return (ecs_event_t){ id };
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
        ECS_MEMSET(start, 0, diff);
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
 * Command queue implementation
 *============================================================================*/
static void ecs_cmd_array_init(ecs_t* ecs, ecs_cmd_array_t* queue, size_t capacity)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(queue));
    ECS_ASSERT(capacity > 0);

    (void)ecs;

    queue->size     = 0;
    queue->capacity = capacity;
    queue->data     = (ecs_cmd_t*)ECS_MALLOC(capacity * sizeof(ecs_cmd_t), ecs->mem_ctx);

    ECS_ASSERT(ecs_is_not_null(queue->data));
}

static void ecs_cmd_array_free(ecs_t* ecs, ecs_cmd_array_t* queue)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(queue));
    ECS_FREE(queue->data, ecs->mem_ctx);
    (void)ecs;
    (void)queue;
}

static ecs_cmd_t* ecs_cmd_array_push(ecs_t* ecs, ecs_cmd_array_t* queue)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(queue));

    (void)ecs;

    if (queue->size == queue->capacity)
    {
        size_t new_capacity = queue->capacity * 2;

        ECS_ASSERT(ecs_is_valid_capacity(new_capacity, sizeof(ecs_cmd_t)));
        queue->data = (ecs_cmd_t*)ECS_REALLOC(queue->data,
                                              new_capacity * sizeof(ecs_cmd_t),
                                              ecs->mem_ctx);
        queue->capacity = new_capacity;
    }

    ecs_cmd_t* cmd = &queue->data[queue->size++];
    ECS_MEMSET(cmd, 0, sizeof(ecs_cmd_t));
    return cmd;
}

static void ecs_cmd_flush_queue(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    ecs_cmd_array_t* queue = &ecs->cmd_queue;

    for (size_t i = 0; i < queue->size; ++i)
    {
        ecs_cmd_t* cmd = &queue->data[i];

        switch (cmd->type)
        {
            case ECS_CMD_SET:
                if (ecs_is_ready(ecs, cmd->entity))
                {
                    ecs_set(ecs, cmd->entity, cmd->comp, cmd->data);
                }
                break;

            case ECS_CMD_ADD:
                if (ecs_is_ready(ecs, cmd->entity))
                {
                    ecs_bitset_flip(&ecs->entities[cmd->entity.id].comp_bits, cmd->comp.id, false);
                    ecs_add(ecs, cmd->entity, cmd->comp, cmd->args);
                }
                break;

            case ECS_CMD_REMOVE:
                if (ecs_is_ready(ecs, cmd->entity))
                {
                    ecs_bitset_flip(&ecs->entities[cmd->entity.id].comp_bits, cmd->comp.id, true);
                    ecs_remove(ecs, cmd->entity, cmd->comp);
                }
                break;

            case ECS_CMD_DESTROY:
                if (ecs_is_active(ecs, cmd->entity.id))
                {
                    ecs->entities[cmd->entity.id].ready = true;
                    ecs_destroy(ecs, cmd->entity);
                }
                break;
        }
    }

    queue->size = 0;
}

/*=============================================================================
 * Event queue implementation
 *============================================================================*/
static void ecs_event_queue_init(ecs_t* ecs, ecs_event_queue_t* queue, size_t capacity)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(queue));
    ECS_ASSERT(capacity > 0);

    queue->size     = 0;
    queue->capacity = capacity;
    queue->data     = (ecs_event_msg_t*)ECS_MALLOC(capacity * sizeof(ecs_event_msg_t),
                                                   ecs->mem_ctx);

    ECS_ASSERT(ecs_is_not_null(queue->data));
}

static void ecs_event_queue_free(ecs_t* ecs, ecs_event_queue_t* queue)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(queue));
    ECS_FREE(queue->data, ecs->mem_ctx);
    (void)ecs;
    (void)queue;
}

static ecs_event_msg_t* ecs_event_queue_push(ecs_t* ecs, ecs_event_queue_t* queue)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(queue));

    (void)ecs;

    if (queue->size == queue->capacity)
    {
        size_t new_capacity = queue->capacity * 2;

        ECS_ASSERT(ecs_is_valid_capacity(new_capacity, sizeof(ecs_event_msg_t)));
        queue->data = (ecs_event_msg_t*)ECS_REALLOC(queue->data,
                                                    new_capacity * sizeof(ecs_event_msg_t),
                                                    ecs->mem_ctx);
        queue->capacity = new_capacity;
    }

    ecs_event_msg_t* msg = &queue->data[queue->size++];
    ECS_MEMSET(msg, 0, sizeof(ecs_event_msg_t));
    return msg;
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

static ecs_arena_block_t* ecs_arena_block_create(ecs_t* ecs, size_t size)
{
    (void)ecs;

    ecs_arena_block_t* block = (ecs_arena_block_t*)ECS_MALLOC(sizeof(ecs_arena_block_t), ecs->mem_ctx);

    if (!block)
        return NULL;

    block->memory = (uint8_t*)ECS_MALLOC(size, ecs->mem_ctx);

    if (!block->memory)
    {
        ECS_FREE(block, ecs->mem_ctx);
        return NULL;
    }

    block->size   = size;
    block->offset = 0;
    block->next   = NULL;

    return block;
}

static bool ecs_arena_init(ecs_t* ecs, ecs_arena_t* arena, size_t initial_block_size)
{
    ecs_arena_block_t* block = ecs_arena_block_create(ecs, initial_block_size);

    if (!block)
        return false;

    arena->first      = block;
    arena->current    = block;
    arena->block_size = initial_block_size;

    return true;
}

static bool ecs_arena_grow(ecs_t* ecs, ecs_arena_t* arena, size_t min_size)
{
    size_t new_size = arena->block_size;

    while (new_size < min_size)
    {
        new_size *= 2;
    }

    ecs_arena_block_t* block = ecs_arena_block_create(ecs, new_size);

    if (!block)
        return false;

    arena->current->next = block;
    arena->current       = block;

    return true;
}

static uintptr_t ecs_arena_align_forward(uintptr_t ptr, size_t align)
{
    uintptr_t mask = (uintptr_t)align - 1;
    return (ptr + mask) & ~mask;
}

static void* ecs_arena_alloc_align(ecs_t* ecs, ecs_arena_t* arena, size_t size, size_t align)
{
    if ((align & (align - 1)) != 0)
        return NULL; // align must be a power of two

    ecs_arena_block_t* block = arena->current;

    uintptr_t base      = (uintptr_t)block->memory;
    uintptr_t ptr       = base + block->offset;
    uintptr_t aligned   = ecs_arena_align_forward(ptr, align);
    size_t    new_offset = (aligned - base) + size;

    if (new_offset > block->size)
    {
        size_t required = size + align;

        if (!ecs_arena_grow(ecs, arena, required))
            return NULL;

        block      = arena->current;
        base       = (uintptr_t)block->memory;
        aligned    = ecs_arena_align_forward(base, align);
        new_offset = (aligned - base) + size;
    }

    block->offset = new_offset;

    return (void*)aligned;
}

static void* ecs_arena_alloc(ecs_t* ecs, ecs_arena_t* arena, size_t size)
{
    return ecs_arena_alloc_align(ecs, arena, size, alignof(max_align_t));
}

static void ecs_arena_reset(ecs_t* ecs, ecs_arena_t* arena)
{
    (void)ecs;

    ecs_arena_block_t* block = arena->first->next;

    while (block)
    {
        ecs_arena_block_t* next = block->next;
        ECS_FREE(block->memory, ecs->mem_ctx);
        ECS_FREE(block, ecs->mem_ctx);
        block = next;
    }

    arena->first->next   = NULL;
    arena->first->offset = 0;
    arena->current       = arena->first;
}

static void ecs_arena_destroy(ecs_t* ecs, ecs_arena_t* arena)
{
    (void)ecs;

    ecs_arena_block_t* block = arena->first;

    while (block)
    {
        ecs_arena_block_t* next = block->next;
        ECS_FREE(block->memory, ecs->mem_ctx);
        ECS_FREE(block, ecs->mem_ctx);
        block = next;
    }

    arena->first   = NULL;
    arena->current = NULL;
}

/*=============================================================================
 * Sparse set functions
 *============================================================================*/

static void ecs_sparse_set_init(ecs_t* ecs, ecs_sparse_set_t* set, size_t capacity)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(set));

    (void)ecs;

    set->capacity = capacity;
    set->size = 0;

    ECS_ASSERT(ecs_is_valid_capacity(capacity, sizeof(ecs_entity_t)));
    set->dense  = (ecs_entity_t*)ECS_MALLOC(capacity * sizeof(ecs_entity_t), ecs->mem_ctx);

    ECS_ASSERT(ecs_is_valid_capacity(capacity, sizeof(size_t)));
    set->sparse = (size_t*)      ECS_MALLOC(capacity * sizeof(size_t),   ecs->mem_ctx);

    ECS_MEMSET(set->sparse, 0, capacity * sizeof(size_t));
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
    ECS_ASSERT(ecs_is_valid_id(id));

    (void)ecs;

    // Check if ID exists within the set
    if (ecs_sparse_set_find(set, id, NULL))
        return false;

    // Grow sparse set if necessary
    if (id >= set->capacity)
    {
        size_t old_capacity = set->capacity;
        size_t new_capacity = old_capacity;

        // Note that since a valid id doesn't have its high bit set, and
        // capacity is in terms of elements, doubling the capacity won't wrap
        do {
            new_capacity *= 2;
        } while (id >= new_capacity);


        // Grow dense array
        ECS_ASSERT(ecs_is_valid_capacity(set->capacity, sizeof(ecs_entity_t)));
        set->dense = (ecs_entity_t*)ecs_realloc_zero(ecs,
                                                set->dense,
                                                old_capacity * sizeof(ecs_entity_t),
                                                new_capacity * sizeof(ecs_entity_t));


        // Grow sparse array and zero it
        ECS_ASSERT(ecs_is_valid_capacity(set->capacity, sizeof(size_t)));
        set->sparse = (size_t*)ecs_realloc_zero(ecs,
                                                set->sparse,
                                                old_capacity * sizeof(size_t),
                                                new_capacity * sizeof(size_t));

        // Set the new capacity
        set->capacity = new_capacity;
    }

    // Add ID to set
    set->dense[set->size].id = id;
    set->sparse[id] = set->size;
    set->size++;

    return true;
}

static inline bool ecs_sparse_set_find(ecs_sparse_set_t* set, ecs_id_t id, size_t* found)
{
    ECS_ASSERT(ecs_is_not_null(set));
    ECS_ASSERT(ecs_is_valid_id(id));

    if (id < set->capacity && set->sparse[id] < set->size && set->dense[set->sparse[id]].id == id)
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

static inline bool ecs_sparse_set_remove(ecs_sparse_set_t* set, ecs_id_t id)
{
    ECS_ASSERT(ecs_is_not_null(set));
    ECS_ASSERT(ecs_is_valid_id(id));

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
#if ECS_MAX_COMPONENTS <= 64

static inline bool ecs_entity_system_test(ecs_bitset_t require_bits,
                                          ecs_bitset_t exclude_bits,
                                          ecs_bitset_t entity_bits)
{
    if (entity_bits & exclude_bits)
        return false;

    if ((entity_bits & require_bits) != require_bits)
        return false;

    return true;
}

#else // ECS_MAX_COMPONENTS

static inline bool ecs_entity_system_test(ecs_bitset_t require_bits,
                                          ecs_bitset_t exclude_bits,
                                          ecs_bitset_t entity_bits)
{
    if (!ecs_bitset_is_zero(&exclude_bits))
    {
        ecs_bitset_t overlap = ecs_bitset_and(&entity_bits, &exclude_bits);

        if (ecs_bitset_true(&overlap))
        {
            return false;
        }
    }

    ecs_bitset_t entity_and_require = ecs_bitset_and(&entity_bits, &require_bits);
    return ecs_bitset_equal(&entity_and_require, &require_bits);
}
#endif // ECS_MAX_COMPONENTS

// True if a built-in lifecycle event has a user listener. The built-in
// forwarding listener is always subscribed first (slot 0) and never removed, so
// any active listener at a later slot is a user subscription (via the
// ecs_get_*_event handles). The lifecycle emit sites use this so raw subscribers
// receive events even for systems/components without a convenience callback,
// while still emitting nothing when nobody is listening at all.
static inline bool ecs_event_has_user_listener(ecs_t* ecs, ecs_event_t event)
{
    ecs_event_data_t* event_data = &ecs->events[event.id];

    for (size_t i = 1; i < event_data->listener_count; i++)
    {
        if (event_data->listeners[i].active)
            return true;
    }

    return false;
}

static void ecs_sync_add_remove(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id)
{
    // Load entity data
    ecs_entity_data_t* entity_data = &ecs->entities[entity_id];

    // Add or remove entity from systems
    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++)
    {
        ecs_sys_data_t* sys_data = &ecs->systems[sys_id];

        // Skip systems that don't reference the changed component --
        // their match result cannot have changed
        if (!ecs_bitset_test(&sys_data->require_bits, comp_id) &&
            !ecs_bitset_test(&sys_data->exclude_bits, comp_id))
            continue;

        // Test to see if entity's components matches the system
        if (ecs_entity_system_test(sys_data->require_bits,
                                   sys_data->exclude_bits,
                                   entity_data->comp_bits))
        {
            // Add the entity directly to the sparse set
            if (ecs_sparse_set_add(ecs, &sys_data->entity_ids, entity_id) &&
                (sys_data->on_join || ecs_event_has_user_listener(ecs, ecs->join_event)))
            {
                // Queue the built-in join event, sourced from this system
                ecs_sys_event_t msg = { ecs_make_entity(entity_id), ecs_make_system(sys_id) };
                ecs_enqueue_impl(ecs, ecs->join_event, sys_id, &msg);
            }
        }
        else
        {
            // Just remove the entity from the sparse set if its components
            // no longer match
            if (ecs_sparse_set_remove(&sys_data->entity_ids, entity_id) &&
                (sys_data->on_leave || ecs_event_has_user_listener(ecs, ecs->leave_event)))
            {
                // Queue the built-in leave event, sourced from this system
                ecs_sys_event_t msg = { ecs_make_entity(entity_id), ecs_make_system(sys_id) };
                ecs_enqueue_impl(ecs, ecs->leave_event, sys_id, &msg);
            }
        }
    }
}

static void ecs_sync_destroy(ecs_t* ecs, ecs_id_t entity_id)
{
    // Remove entity from systems
    for (ecs_id_t sys_id = 0; sys_id < ecs->system_count; sys_id++)
    {
        ecs_sys_data_t* sys_data = &ecs->systems[sys_id];

        if (ecs_sparse_set_remove(&sys_data->entity_ids, entity_id) &&
            (sys_data->on_leave || ecs_event_has_user_listener(ecs, ecs->leave_event)))
        {
            // Queue the built-in leave event, sourced from this system
            ecs_sys_event_t msg = { ecs_make_entity(entity_id), ecs_make_system(sys_id) };
            ecs_enqueue_impl(ecs, ecs->leave_event, sys_id, &msg);
        }
    }
}

/*=============================================================================
 * ID array functions
 *============================================================================*/

static void ecs_id_array_init(ecs_t* ecs, ecs_id_array_t* array, size_t capacity)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(array));

    (void)ecs;

    array->size = 0;
    array->capacity = capacity;

    ECS_ASSERT(ecs_is_valid_capacity(capacity, sizeof(ecs_id_t)));
    array->data = (ecs_id_t*)ECS_MALLOC(capacity * sizeof(ecs_id_t), ecs->mem_ctx);
}

static void ecs_id_array_free(ecs_t* ecs, ecs_id_array_t* array)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(array));

    (void)ecs;

    ECS_FREE(array->data, ecs->mem_ctx);
}

static inline void ecs_id_array_push(ecs_t* ecs, ecs_id_array_t* array, ecs_id_t id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(array));
    ECS_ASSERT(ecs_is_valid_id(id));

    (void)ecs;

    if (array->size == array->capacity)
    {

        // Note that since a valid id doesn't have its high bit set, and
        // capacity is in terms of elements, doubling the capacity won't wrap
        array->capacity *= 2;

        ECS_ASSERT(ecs_is_valid_capacity(array->capacity, sizeof(ecs_id_t)));
        array->data = (ecs_id_t*)ECS_REALLOC(array->data,
                                             array->capacity * sizeof(ecs_id_t),
                                             ecs->mem_ctx);
    }

    array->data[array->size++] = id;
}

static inline ecs_id_t ecs_id_array_pop(ecs_id_array_t* array)
{
    ECS_ASSERT(ecs_is_not_null(array));
    ECS_ASSERT(array->size > 0);

    return array->data[--array->size];
}

static inline size_t ecs_id_array_size(ecs_id_array_t* array)
{
    return array->size;
}

static void ecs_comp_blocks_init(ecs_t* ecs, ecs_comp_blocks_t* array, size_t size, size_t capacity)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(array));

    (void)ecs;

    ECS_MEMSET(array, 0, sizeof(ecs_comp_blocks_t));

    array->comp_size = size;

    size_t initial_blocks = (capacity + ECS_COMP_BLOCK_SIZE - 1) / ECS_COMP_BLOCK_SIZE;
    if (initial_blocks == 0) initial_blocks = 1;

    array->block_capacity = initial_blocks;
    array->block_count    = initial_blocks;

    ECS_ASSERT(ecs_is_valid_capacity(initial_blocks, sizeof(void*)));
    array->blocks = (void**)ECS_MALLOC(initial_blocks * sizeof(void*), ecs->mem_ctx);

    for (size_t i = 0; i < initial_blocks; i++)
    {
        ECS_ASSERT(ecs_is_valid_capacity(ECS_COMP_BLOCK_SIZE, size));
        array->blocks[i] = ECS_MALLOC(ECS_COMP_BLOCK_SIZE * size, ecs->mem_ctx);
        ECS_MEMSET(array->blocks[i], 0, ECS_COMP_BLOCK_SIZE * size);
    }
}

static void ecs_comp_blocks_free(ecs_t* ecs, ecs_comp_blocks_t* array)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(array));

    (void)ecs;

    for (size_t i = 0; i < array->block_count; i++)
    {
        ECS_FREE(array->blocks[i], ecs->mem_ctx);
    }

    ECS_FREE(array->blocks, ecs->mem_ctx);
}

static void ecs_comp_blocks_resize(ecs_t* ecs, ecs_comp_blocks_t* array, ecs_id_t id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(array));
    ECS_ASSERT(ecs_is_valid_id(id));

    size_t required_block = id / ECS_COMP_BLOCK_SIZE;

    while (required_block >= array->block_count)
    {
        // Grow the block pointer array if necessary. This only moves pointers,
        // never the block data itself, so existing component pointers remain valid.
        if (array->block_count == array->block_capacity)
        {
            size_t old_capacity = array->block_capacity;
            size_t new_capacity = old_capacity * 2;

            ECS_ASSERT(ecs_is_valid_capacity(array->block_capacity, sizeof(void*)));
            array->blocks = (void**)ecs_realloc_zero(ecs,
                                                     array->blocks,
                                                     old_capacity * sizeof(void*),
                                                     new_capacity * sizeof(void*));


            array->block_capacity = new_capacity;
        }

        // Allocate and zero a new block
        ECS_ASSERT(ecs_is_valid_capacity(ECS_COMP_BLOCK_SIZE, array->comp_size));
        void* block = ECS_MALLOC(ECS_COMP_BLOCK_SIZE * array->comp_size, ecs->mem_ctx);
        ECS_MEMSET(block, 0, ECS_COMP_BLOCK_SIZE * array->comp_size);
        array->blocks[array->block_count++] = block;
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

static bool ecs_is_valid_event_id(ecs_id_t id)
{
    return id < ECS_MAX_EVENTS;
}

static bool ecs_is_valid_id(ecs_id_t id)
{
    // Ensures high bit is not set - works for any unsigned ecs_id_t
    return id == ((id << 1) >> 1);
}

static bool ecs_is_valid_capacity(size_t capacity, size_t elem_size)
{
    // Ensures any array allocations won't overflow a signed size_t and are
    // nonzero. This is not the most efficient implementation, but it is simple

    if (capacity == 0 || elem_size == 0)
    {
        return false;
    }

    size_t max_cap = (SIZE_MAX >> 1) / elem_size;
    return capacity <= max_cap;
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

static bool ecs_is_event_ready(ecs_t* ecs, ecs_id_t event_id)
{
    return event_id < ecs->event_count;
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
