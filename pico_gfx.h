/**
    @file pico_gfx.h
    @brief A powerful graphics library based on Sokol GFX, written in C99.

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Features:
    ---------

    - Written in C99
    - Single header library for easy build system integration
    - Easy to use Low-level constructs (render passes and pipelines)
    - Simple texture and shader creation
    - Default shader and uniform block
    - Render to texture
    - Rendering of dynamic vertex arrays, indexed vertex arrays
    - Rendering of static vertex buffers
    - Simple API for managing uniform blocks
    - Straight foward state management (state stack)
    - Simple and concise API
    - Permissive license (zlib or public domain)

    Summary:
    --------

    Pico GFX (pico_gfx) is a high-level wrapper for [sokol_gfx](https://github.com/floooh/sokol/blob/master/sokol_gfx.h),
    a low-level wrapper for OpenGL, Metal, and D3D. Pico GFX is designed to make
    the common case intuitive and convenient, especially for 2D applications. It
    provides access to low-level constructs, such as render passes and
    pipelines, in a way that is easy to use and understand.

    pico_gfx includes a default shader (and pipeline), but can be extended using
    the sokol shader compiler (`sokol-shdc`) which allows for a shader to be
    written in a single language (e.g. GLSL) which is then transformed into
    shader sources for all suppported backends.

    One thing pico_gfx does not support (and neither does sokol_gfx) is window
    and graphics context creation. See [here](https://github.com/RandyGaul/cute_framework/tree/master/src/internal)
    for some examples. It is worth mentioning that [SDL2](https://www.libsdl.org)
    can supply both a window and OpenGL context out of the box. SDL2 is used
    in the demos.

    Another library that supports window/context creation on all supported
    backends is [sokol_app](https://github.com/floooh/sokol/blob/master/sokol_app.h),
    but it has yet to be tested with pico_gfx.

    State (pipeline/shader, the default uniform block, the viewport, scissor,
    and clear color) can be managed via the state stack. The stack enables
    changes to be isolated. Simply push the current state to the top of the
    stack, make some local changes, and then pop the stack to restore the
    original state.

    Shaders expose uniforms in blocks. These blocks must be registered with the
    shader by calling `pg_register_uniform_block`. They may then be set at will
    by calling `pg_set_uniform_block`. These functions typically operate on
    structs supplied by a custom shader,

    The default shader provides a uniform block containing a projection and
    transformation matrix that are used to map vertices to normalized device
    coordinates. This projection matrix can be set using `pg_set_projection`
    and the (model-view) transformation matrix can be set using
    `pg_set_transform`. They can be reset to the identity by calling
    `pg_reset_projection` and `pg_reset_transform` respectively.

    Please see the examples for more details.

    Build:
    --------

    To use this library in your project, add

    > #define PICO_GFX_IMPLEMENTATION
    > #include "pico_gfx.h"

    to a source file.

    You must also define one of

    #define PICO_GFX_GL
    #define PICO_GFX_GLES
    #define PICO_GFX_D3D
    #define PICO_GFX_METAL
    #define PICO_GFX_WEBGPU

    before including pico_gfx.h

    IMPORTANT: sokol_gfx.h must be in the include path!

    See the examples for build details.

    Constants:
    --------

    - PICO_GFX_STACK_MAX_SIZE (default: 16)
    - PICO_GFX_BUFFER_SIZE (default: 1024)
    - PG_GFX_HT_MIN_CAPACITY (default: 8)
    - PG_GFX_HT_KEY_SIZE (default: 16)
    - PICO_GFX_MIN_ARENA_CAPACITY (default: 1024)

    Customization:
    --------

    - NDEBUG
    - PICO_GFX_ASSERT
    - PICO_GFX_LOG
    - PICO_GFX_MALLOC
    - PICO_GFX_REALLOC
    - PICO_GFX_FREE

    The above (constants/macros) must be defined before PICO_GFX_IMPLEMENTATION
*/

// Backend conversion macros
#if defined (PICO_GFX_GL)
    #define SOKOL_GLCORE
#elif defined (PICO_GFX_GLES)
    #define SOKOL_GLES3
#elif defined (PICO_GFX_D3D)
    #define SOKOL_D3D11
#elif defined (PICO_GFX_METAL)
    #define SOKOL_METAL
#elif defined (PICO_GFX_WEBGPU)
    #define SOKOL_WGPU
#else
    #error "GFX backend must be specified"
#endif

#ifndef PICO_GFX_H
#define PICO_GFX_H

#include "sokol_gfx.h"
#include "pico_gfx_shader.h"

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Graphics backends
*/
typedef enum
{
    PG_BACKEND_GL,
    PG_BACKEND_GLES,
    PG_BACKEND_D3D,
    PG_BACKEND_METAL,
    PG_BACKEND_WGPU
} pg_backend_t;

/**
 * @brief Drawing primitives
 */
typedef enum
{
    PG_DEFAULT_PRIMITIVE,
    PG_POINTS,         //!< Array of points
    PG_LINES,          //!< Each adjacent pair of points forms a line
    PG_LINE_STRIP,     //!< Array of points where every pair forms a lines
    PG_TRIANGLES,      //!< Each adjacent triple forms an individual triangle
    PG_TRIANGLE_STRIP, //!< Array of points where every triple forms a triangle
} pg_primitive_t;

/**
 * @brief Blend factors
 */
typedef enum
{
    PG_DEFAULT_BLEND_FACTOR,
    PG_ZERO,                //!< (0, 0, 0,  0)
    PG_ONE,                 //!< (1, 1, 1, 1)
    PG_SRC_COLOR,           //!< (src.r, src.g, src.b, src.a)
    PG_ONE_MINUS_SRC_COLOR, //!< (1, 1, 1, 1) - (src.r, src.g, src.b, src.a)
    PG_DST_COLOR,           //!< (dst.r, dst.g, dst.b, dst.a)
    PG_ONE_MINUS_DST_COLOR, //!< (1, 1, 1, 1) - (dst.r, dst.g, dst.b, dst.a)
    PG_SRC_ALPHA,           //!< (src.a, src.a, src.a, src.a)
    PG_ONE_MINUS_SRC_ALPHA, //!< (1, 1, 1, 1) - (src.a, src.a, src.a, src.a)
    PG_DST_ALPHA,           //!< (dst.a, dst.a, dst.a, dst.a)
    PG_ONE_MINUS_DST_ALPHA, //!< (1, 1, 1, 1) - (dst.a, dst.a, dst.a, dst.a)
} pg_blend_factor_t;

/**
 * @brief Blend equations
 */
typedef enum
{
    PG_DEFAULT_BLEND_EQ,
    PG_ADD,              //!< result = src * src_factor + dst * dst_factor
    PG_SUBTRACT,         //!< result = src * src_factor - dst * dst_factor
    PG_REVERSE_SUBTRACT, //!< result = dst * dst_factor - src * src_factor
} pg_blend_eq_t;

/**
 * @brief Blend mode
 *
 * Completely describes a blend operation.
 */
typedef struct
{
    pg_blend_factor_t color_src; //!< Color source blending factor
    pg_blend_factor_t color_dst; //!< Color dsestination blending factor
    pg_blend_eq_t     color_eq;  //!< Equation for blending colors
    pg_blend_factor_t alpha_src; //!< Alpha source blending factor
    pg_blend_factor_t alpha_dst; //!< Alpha destination blending factor
    pg_blend_eq_t     alpha_eq;  //!< Equation for blending alpha values
} pg_blend_mode_t;

/**
 * @brief A vertex describes a point and the data associated with it (color and
 * texture coordinates)
 */
typedef struct
{
    float pos[3];
    float color[4];
    float uv[2];
} pg_vertex_t;

/**
 * @brief Shader stage
*/
typedef enum
{
    PG_VS_STAGE, //!< Vertex shader stage
    PG_FS_STAGE  //!< Fragment shader stage
} pg_stage_t;

/**
 * @brief A 4x4 matrix
 */
typedef float pg_mat4_t[16];

/**
 * @brief Contains core data/state for an instance of the graphics library
 */
typedef struct pg_ctx_t pg_ctx_t;

/**
 * @brief Render state information
 */
typedef struct pg_pipeline_t pg_pipeline_t;

/**
 * @brief Vertex/fragment shader program
 */
typedef struct pg_shader_t pg_shader_t;

/**
 * @brief Represents an image or render target in VRAM
 */
typedef struct pg_texture_t pg_texture_t;

/**
 * @brief Represents sampler
 */
typedef struct pg_sampler_t pg_sampler_t;

/**
 * @brief A vertex array buffer
 */
typedef struct pg_vbuffer_t pg_vbuffer_t;

/**
 * @brief Loads pico_gfx and sokol_gfx
 *
 * IMPORTANT: A valid graphics API context (OpenGL, Metal, D3D) must exist for
 * this function to succeed. This function must be called before any other
 * pico_gfx functions.
 *
 * NOTE: This function calls `sg_setup`.
 */
void pg_init(void);

/**
 *  @brief Tears down pico_gfx and sokol_gfx
 *
 * NOTE: This function calls `sg_shutdown`
 */
void pg_shutdown(void);

/**
 * @brief Creates a graphics context
 * @param window_width The window width
 * @param window_height The window height
 */
pg_ctx_t* pg_create_context(int window_width, int window_height, void* mem_ctx);

/**
 * @brief Destroys a graphics context
 */
void pg_destroy_context(pg_ctx_t* ctx);

/**
 * @brief Returns the backend in use at runtime
*/
pg_backend_t pg_backend();

/**
 * @brief Sets the window dimensions
 * @param ctx The graphics context
 * @param width The window width
 * @param height The window height
 * @param reset Resets the viewport and scissor if true
 */
void pg_set_window_size(pg_ctx_t* ctx, int width, int height, bool reset);

/**
 * @brief Gets the window size
 */
void pg_get_window_size(pg_ctx_t* ctx, int* width, int* height);

/**
 * @brief Starts a render pass (mandatory)
 *
 * NOTE: The default pass should be the last pass of a frame.
 *
 * @param ctx The graphics context
 * @param pass The render pass (NULL for the default pass)
 * @param clear Clears the render target or window
 */
void pg_begin_pass(pg_ctx_t* ctx, pg_texture_t* target, bool clear);

/**
 * @brief Ends a render pass (mandatory)
 */
void pg_end_pass(pg_ctx_t* ctx);

/**
 * @brief Flush commands
 *
 * Must be called at the end of a frame (after `pg_end_pass`).
*/
void pg_flush(pg_ctx_t* ctx);

/**
 * @brief Pushes the active state onto the stack.
 *
 * State consists of the pipeline, draw color, scissor, viewport, and default
 * MVP transform.
 */
void pg_push_state(pg_ctx_t* ctx);

/**
 * @brief Pops a state off the stack and makes it the active state
 */
void pg_pop_state(pg_ctx_t* ctx);

/**
 * @brief Sets the clear color state to be placed at the top of the state stack
 */
void pg_set_clear_color(pg_ctx_t* ctx, float r, float g, float b, float a);

/**
 * Resets the clear color
 */
void pg_reset_clear_color(pg_ctx_t* ctx);

/**
 * @brief Sets the viewport state to be placed at the top of the state stack
 */
void pg_set_viewport(pg_ctx_t* ctx, int x, int y, int w, int h);

/**
 * Resets the viewport
 */
void pg_reset_viewport(pg_ctx_t* ctx);

/**
 * @brief Sets the scissor state to be placed at the top of the state stack
 */
void pg_set_scissor(pg_ctx_t* ctx, int x, int y, int w, int h);

/**
 * Resets the scissor
 */
void pg_reset_scissor(pg_ctx_t* ctx);

/**
 * @brief Sets the pipeline state
 * @param ctx The graphics context
 * @param pipeline The pipeline to be placed on the top of the state stack
 */
void pg_set_pipeline(pg_ctx_t* ctx, pg_pipeline_t* pipeline);

/**
 * Resets the pipeline
 */
void pg_reset_pipeline(pg_ctx_t* ctx);

/**
 * @brief Sets the projection matrix
 */
void pg_set_projection(pg_ctx_t* ctx, pg_mat4_t matrix);

/**
 * @brief Resets the projection matrix to the identity
 */
void pg_reset_projection(pg_ctx_t* ctx);

/**
 * @brief Sets the (modelview) transform matrix
 */
void pg_set_transform(pg_ctx_t* ctx, pg_mat4_t matrix);

/**
 * @brief Resets the (modelview) transform matrix to the identity
 */
void pg_reset_transform(pg_ctx_t* ctx);

/**
 * @brief Resets the active state to defaults
 */
void pg_reset_state(pg_ctx_t* ctx);

/**
 * @brief Creates the shader with the given prefix
 * The prefix should refer to the shader program name in a shader compiled by
 * `sokol-shdc`
 */
#define pg_create_shader(ctx, prefix)   \
    pg_create_shader_internal(          \
        ctx,                            \
        (pg_shader_internal_t)          \
        {                               \
            prefix##_shader_desc,       \
            prefix##_uniformblock_slot, \
        }                               \
    )

/**
 * @brief Destroys a shader
 */
void pg_destroy_shader(const pg_ctx_t* ctx, pg_shader_t* shader);

/**
 * @brief Returns the default shader
 */
pg_shader_t* pg_get_default_shader(const pg_ctx_t* ctx);

/**
 * @brief Returns the default pipeline
 */
pg_pipeline_t* pg_get_default_pipeline(const pg_ctx_t* ctx);

/**
 * @brief Returns a shader ID
 */
uint32_t pg_get_shader_id(const pg_shader_t* shader);

/**
 * @brief Registers a uniform block (UB)
 * @param shader The shader owning the UB
 * @param stage The stage (VS or FS) associated with the UB
 * @param name The name of the UB as supplied by `sokol_shdc` (no quotes)
 */
#define pg_register_uniform_block(shader, stage, name) \
        pg_register_uniform_block_internal(shader, stage, #name, sizeof(name##_t))

/**
 * @brief Sets a uniform block (UB)
 * @param shader The shader owning the UB
 * @param name The name of the UB as supplied by `sokol_shdc` (no quotes)
 * @param data The data to set (must be the whole UB)
 */
#define pg_set_uniform_block(shader, name, data) \
        pg_set_uniform_block_internal(shader, #name, data)

/**
 * @brief Pipeline creation options
 */
typedef struct pg_pipeline_opts_t
{
    pg_primitive_t primitive; //!< Rendering primitive
    bool target;              //!< Drawing to render target
    bool indexed;             //!< Indexed drawing
    bool blend_enabled;       //!< Enables blending
    pg_blend_mode_t blend;    //!< Blend mode
} pg_pipeline_opts_t;

/**
 * @brief Creates a rendering pipeline (encapsulates render state)
 * @param shader The shader used by this pipeline
 * @param opts Pipeline creation options
 * @returns A render pipeline object
 */
pg_pipeline_t* pg_create_pipeline(const pg_ctx_t* ctx,
                                  pg_shader_t* shader,
                                  const pg_pipeline_opts_t* opts);

/**
 * @brief Destroys a render pipeline
*/
void pg_destroy_pipeline(const pg_ctx_t* ctx, pg_pipeline_t* pipeline);

/**
 * @brief Texture creation options
 */
typedef struct pg_texture_opts_t
{
    int mipmaps; //!< Mipmap level
} pg_texture_opts_t;

/**
 * @brief Creates a texture from an RGBA8 image
 * @param width Image width
 * @param height Image height
 * @param data Image data (format must be RGBA8)
 * @param size Size of the data in bytes
 * @param opts Texture creation options (NULL for defaults)
 * @returns A texture created from a bitmap
 */
pg_texture_t* pg_create_texture(const pg_ctx_t* ctx,
                                int width, int height,
                                const uint8_t* data, size_t size,
                                const pg_texture_opts_t* opts);

/**
 * @brief Creates a render target
 * @param width Render target width
 * @param height Render target height
 * @param opts Texture creation options (NULL for defaults)
 * @returns A render texture
 */
pg_texture_t* pg_create_render_texture(const pg_ctx_t* ctx,
                                       int width, int height,
                                       const pg_texture_opts_t* opts);

/**
 * @brief Destroys a texture
 */
void pg_destroy_texture(const pg_ctx_t* ctx, pg_texture_t* texture);

void pg_bind_texture(pg_ctx_t* ctx, int slot, pg_texture_t* texture);

void pg_reset_textures(pg_ctx_t* ctx);

/**
 * @brief Returns a texture ID
 */
uint32_t pg_get_texture_id(const pg_texture_t* texture);

/**
 * @brief Gets a texture's dimensions
 */
void pg_get_texture_size(const pg_texture_t* texture, int* width, int* height);

typedef struct
{
    bool smooth; //!< Linear filtering if true, nearest otherwise
    bool repeat; //!< Repeat if true, clamp-to-edge otherwise
} pg_sampler_opts_t;

pg_sampler_t* pg_create_sampler(const pg_ctx_t* ctx,
                                const pg_sampler_opts_t* opts);

void pg_destroy_sampler(const pg_ctx_t* ctx, pg_sampler_t* sampler);

void pg_bind_sampler(pg_ctx_t* ctx, int slot, pg_sampler_t* sampler);

void pg_reset_samples(pg_ctx_t* ctx);

/**
 * @brief Creates a vertex buffer
 * @param vertices An array of vertices (position, color, uv)
 * @param count The number of vertices
 */
pg_vbuffer_t* pg_create_vbuffer(const pg_ctx_t* ctx,
                                const pg_vertex_t* vertices,
                                size_t count);

/**
 * @brief Destroys a vertex buffer
 */
void pg_destroy_vbuffer(const pg_ctx_t* ctx, pg_vbuffer_t* buffer);

/**
 * @brief Draws a vertex buffer
 * @param ctx The graphics context
 * @param buffer The vertex buffer
 * @param start The first vertex to draw
 * @param count The number of vertices to draw
 * @param texture The texture to draw from
 */
void pg_draw_vbuffer(const pg_ctx_t* ctx,
                     const pg_vbuffer_t* buffer,
                     size_t start, size_t count);

/**
 * @brief Draws an array of vertices
 * @param ctx The graphics context
 * @param vertices An array of vertices (position, color, uv)
 * @param count The number of vertices
 * @param texture The texture to draw from
 */
void pg_draw_array(pg_ctx_t* ctx, const pg_vertex_t* vertices, size_t count);

/**
 * @brief Draws an indexed array of vertices
 * @param ctx The graphics context
 * @param vertices An array of vertices (position, color, uv)
 * @param vertex_count The number of vertices
 * @param indices An array that indexes into the vertex array
 * @param index_count The number of indices
 * @param texture The texture to draw from
 */
void pg_draw_indexed_array(pg_ctx_t* ctx,
                           const pg_vertex_t* vertices, size_t vertex_count,
                           const uint32_t* indices, size_t index_count);

/*=============================================================================
 * Internals
 *============================================================================*/

typedef struct
{
	const sg_shader_desc* (*get_shader_desc)(sg_backend backend);
	int (*get_uniformblock_slot)(sg_shader_stage stage, const char* ub_name);
} pg_shader_internal_t;

pg_shader_t* pg_create_shader_internal(const pg_ctx_t* ctx, pg_shader_internal_t internal);

void pg_register_uniform_block_internal(pg_shader_t* shader,
                                        pg_stage_t stage,
                                        const char* name,
                                        size_t size);

void pg_set_uniform_block_internal(pg_shader_t* shader,
                                   const char* name,
                                   const void* data);

#ifdef __cplusplus
}
#endif

#endif // PICO_GFX_H

/*=============================================================================
 * Implementation
 *============================================================================*/

#ifdef PICO_GFX_IMPLEMENTATION

#include <string.h>

/*=============================================================================
 * Constants
 *============================================================================*/

#ifndef PICO_GFX_STACK_MAX_SIZE
#define PICO_GFX_STACK_MAX_SIZE 16
#endif

#ifndef PICO_GFX_BUFFER_SIZE
#define PICO_GFX_BUFFER_SIZE 16384
#endif

#ifndef PG_GFX_HT_MIN_CAPACITY
#define PG_GFX_HT_MIN_CAPACITY 16
#endif

#ifndef PG_GFX_HT_KEY_SIZE
#define PG_GFX_HT_KEY_SIZE 16
#endif

#ifndef PICO_GFX_MIN_ARENA_CAPACITY
#define PICO_GFX_MIN_ARENA_CAPACITY 512
#endif

#ifndef PICO_GFX_MAX_TEXTURE_SLOTS
#define PICO_GFX_MAX_TEXTURE_SLOTS 16
#endif

#ifndef PICO_GFX_MAX_SAMPLER_SLOTS
#define PICO_GFX_MAX_SAMPLER_SLOTS 16
#endif

/*=============================================================================
 * Macros
 *============================================================================*/

#ifdef NDEBUG
    #define PICO_GFX_ASSERT(expr) ((void)0)
#else
    #ifndef PICO_GFX_ASSERT
        #include <assert.h>
        #define PICO_GFX_ASSERT(expr) (assert(expr))
    #endif
#endif

#if !defined(PICO_GFX_MALLOC) || !defined(PICO_GFX_REALLOC) || !defined(PICO_GFX_FREE)
#include <stdlib.h>
#define PICO_GFX_MALLOC(size, ctx)       (malloc(size))
#define PICO_GFX_REALLOC(ptr, size, ctx) (realloc(ptr, size))
#define PICO_GFX_FREE(ptr, ctx)          (free(ptr))
#endif

#ifndef PICO_GFX_LOG
    #include <stdio.h>
    #define  PICO_GFX_LOG(...) (pg_log(__VA_ARGS__))
#endif

/*=============================================================================
 * GFX Static Funtions
 *============================================================================*/

static sg_primitive_type pg_map_primitive(pg_primitive_t primitive);
static sg_blend_factor pg_map_blend_factor(pg_blend_factor_t factor);
static sg_blend_op pg_map_blend_eq(pg_blend_eq_t eq);
static sg_shader_stage pg_map_stage(pg_stage_t stage);

static void pg_log_sg(const char* tag,              // e.g. 'sg'
                      uint32_t log_level,           // 0=panic, 1=error, 2=warn, 3=info
                      uint32_t log_item_id,         // SG_LOGITEM_*
                      const char* message_or_null,  // a message string, may be nullptr in release mode
                      uint32_t line_nr,             // line number in sokol_gfx.h
                      const char* filename_or_null, // source filename, may be nullptr in release mode
                      void* user_data);

static void pg_log(const char* fmt, ...);

/*=============================================================================
 * Utility
 *============================================================================*/

static bool pg_str_equal(const char* s1, const char* s2)
{
    return strcmp(s1, s2) == 0;
}

/*=============================================================================
 * Hashtable Declarations
 *============================================================================*/

#ifdef PICO_GFX_32BIT
    typedef uint32_t pg_hash_t;
#else
    typedef uint64_t pg_hash_t;
#endif

typedef struct pg_hashtable_t pg_hashtable_t;
typedef struct pg_hashtable_iterator_t pg_hashtable_iterator_t;

typedef void (*pg_hashtable_iterator_fn)(pg_hashtable_iterator_t* iterator,
                                         char* key,
                                         void* value);

static pg_hashtable_t* pg_hashtable_new(size_t capacity, size_t key_size,
                                        size_t value_size, void* mem_ctx);

static void pg_hashtable_free(pg_hashtable_t* ht);

static void pg_hashtable_init_iterator(const pg_hashtable_t* ht,
                                       pg_hashtable_iterator_t* iterator);

static bool pg_hashtable_iterator_next(pg_hashtable_iterator_t* iterator,
                                       char** key, void** value);

static void pg_hashtable_put(pg_hashtable_t* ht,
                             const char* key,
                             const void* value);

static void* pg_hashtable_get(const pg_hashtable_t* ht, const char* key);

struct pg_hashtable_iterator_t
{
    const pg_hashtable_t* ht;
    size_t index;
    size_t count;
};

/*=============================================================================
 * Arena Allocator Declarations
 *============================================================================*/

typedef struct pg_arena_t pg_arena_t;

static pg_arena_t* pg_arena_new(size_t size, void* mem_ctx);
static void* pg_arena_alloc(pg_arena_t* arena, size_t size);
static void pg_arena_free(pg_arena_t* arena);

/*=============================================================================
 * Default Shader Reflection Function Declarations
 *============================================================================*/

const sg_shader_desc* pg_default_shader_desc(sg_backend backend);
int pg_default_uniformblock_slot(sg_shader_stage stage, const char* ub_name);

/*=============================================================================
 * GFX Public API Implementation
 *============================================================================*/

static void* pg_malloc(size_t size, void* ctx)
{
    (void)ctx;
    return PICO_GFX_MALLOC(size, ctx);
}

static void pg_free(void* ptr, void* ctx)
{
    (void)ctx;
    PICO_GFX_FREE(ptr, ctx);
}

typedef struct pg_rect_t
{
    int x, y, width, height;
} pg_rect_t;

typedef struct pg_state_t
{
    sg_color       clear_color;
    pg_pipeline_t* pipeline;
    pg_rect_t      viewport;
    pg_rect_t      scissor;
    pg_shader_t*   shader;
    pg_vs_block_t  vs_block;
    pg_texture_t*  textures[PICO_GFX_MAX_TEXTURE_SLOTS];
    pg_sampler_t*  samplers[PICO_GFX_MAX_SAMPLER_SLOTS];
} pg_state_t;

struct pg_ctx_t
{
    void* mem_ctx;
    sg_swapchain swapchain;
    int window_width;
    int window_height;
    bool indexed;
    sg_buffer buffer;
    sg_buffer index_buffer;
    bool pass_active;
    pg_texture_t* target;
    sg_pass default_pass;
    pg_shader_t* default_shader;
    pg_pipeline_t* default_pipeline;
    pg_state_t state;
    pg_state_t state_stack[PICO_GFX_STACK_MAX_SIZE];
    int stack_size;
};

struct pg_pipeline_t
{
    sg_pipeline handle;
    bool indexed;
    pg_shader_t* shader;
};

struct pg_shader_t
{
    const sg_shader_desc* desc;
    sg_shader handle;
    pg_shader_internal_t internal;
    pg_hashtable_t* uniform_blocks;
    pg_arena_t* arena;
};

typedef struct
{
    int        slot;
    pg_stage_t stage;
    void*      data;
    size_t     size;
    bool       dirty;
} pg_uniform_block_t;

struct pg_texture_t
{
    int width, height;
    bool target;
    sg_image handle;
    sg_image depth_handle;
    sg_attachments attachments;
};

struct pg_sampler_t
{
    sg_sampler handle;
};

struct pg_vbuffer_t
{
    sg_buffer handle;
    size_t count;
};

void pg_init(void)
{
    sg_setup(&(sg_desc)
    {
        .logger.func = pg_log_sg,
        .allocator =
        {
            .alloc_fn = pg_malloc,
            .free_fn = pg_free,
            .user_data = NULL,
        },
        .environment.defaults.color_format = SG_PIXELFORMAT_RGBA8,
    });
}

void pg_shutdown(void)
{
    sg_shutdown();
}

pg_ctx_t* pg_create_context(int window_width, int window_height, void* mem_ctx)
{
    pg_ctx_t* ctx = (pg_ctx_t*)PICO_GFX_MALLOC(sizeof(pg_ctx_t), mem_ctx);

    if (!ctx) return NULL;

    memset(ctx, 0, sizeof(pg_ctx_t));

    ctx->mem_ctx = mem_ctx;
    ctx->window_width  = window_width;
    ctx->window_height = window_height;
    ctx->default_shader = pg_create_shader(ctx, pg_default);
    ctx->default_pipeline = pg_create_pipeline(ctx, ctx->default_shader, NULL);

    pg_register_uniform_block(ctx->default_shader, PG_VS_STAGE, pg_vs_block);

    pg_reset_state(ctx);

    ctx->buffer = sg_make_buffer(&(sg_buffer_desc)
    {
        .type  = SG_BUFFERTYPE_VERTEXBUFFER,
        .size  = PICO_GFX_BUFFER_SIZE * sizeof(pg_vertex_t),
        .usage = SG_USAGE_STREAM
    });

    PICO_GFX_ASSERT(sg_query_buffer_state(ctx->buffer) == SG_RESOURCESTATE_VALID);

    ctx->index_buffer = sg_make_buffer(&(sg_buffer_desc)
    {
        .type  = SG_BUFFERTYPE_INDEXBUFFER,
        .size  = PICO_GFX_BUFFER_SIZE * sizeof(uint32_t),
        .usage = SG_USAGE_STREAM
    });

    PICO_GFX_ASSERT(sg_query_buffer_state(ctx->index_buffer) == SG_RESOURCESTATE_VALID);

    ctx->swapchain = (sg_swapchain)
    {
        .width = window_width,
        .height = window_height,
    };

    return ctx;
}

void pg_destroy_context(pg_ctx_t* ctx)
{
    PICO_GFX_ASSERT(ctx);

    sg_destroy_buffer(ctx->buffer);
    sg_destroy_buffer(ctx->index_buffer);

    pg_destroy_pipeline(ctx, ctx->default_pipeline);
    pg_destroy_shader(ctx, ctx->default_shader);

    PICO_GFX_FREE(ctx, ctx->mem_ctx);
}

pg_backend_t pg_backend()
{
    #if defined (PICO_GFX_GL)
        return PG_BACKEND_GL;
    #elif defined (PICO_GFX_GLES)
        return PG_BACKEND_GLES;
    #elif defined (PICO_GFX_D3D)
        return PG_BACKEND_D3D;
    #elif defined (PICO_GFX_METAL)
        return PG_BACKEND_METAL;
    #elif defined (PICO_GFX_WEBGPU)
        return PG_BACKEND_WGPU;
    #else
        #error "Unknown GFX backend"
    #endif
}

void pg_set_window_size(pg_ctx_t* ctx, int width, int height, bool reset)
{
    ctx->swapchain.width  = ctx->window_width  = width;
    ctx->swapchain.height = ctx->window_height = height;

    if (reset)
    {
        pg_reset_viewport(ctx);
        pg_reset_scissor(ctx);
    }
}

void pg_get_window_size(pg_ctx_t* ctx, int* width, int* height)
{
    if (width)
        *width = ctx->window_width;

    if (height)
        *height = ctx->window_height;
}

void pg_begin_pass(pg_ctx_t* ctx, pg_texture_t* target, bool clear)
{
    PICO_GFX_ASSERT(ctx);
    PICO_GFX_ASSERT(!ctx->pass_active);

    sg_pass_action action = { 0 };

    if (clear)
    {
        sg_color color = ctx->state.clear_color;

        action.colors[0] = (sg_color_attachment_action)
        {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = color
        };
    }

    sg_pass pass = { .action = action };

    if (target)
    {
        pass.attachments = target->attachments;
        ctx->target = target;
    }
    else
    {
        pass.swapchain = ctx->swapchain;
    }

    sg_begin_pass(&pass);

    pg_reset_viewport(ctx);
    pg_reset_scissor(ctx);

    ctx->pass_active = true;
}

void pg_end_pass(pg_ctx_t* ctx)
{
    sg_end_pass();
    ctx->target = NULL;
    ctx->pass_active = false;
}

void pg_flush(pg_ctx_t* ctx)
{
    PICO_GFX_ASSERT(ctx);

    sg_commit();

    sg_destroy_buffer(ctx->buffer);

    ctx->buffer = sg_make_buffer(&(sg_buffer_desc)
    {
        .type  = SG_BUFFERTYPE_VERTEXBUFFER,
        .size  = PICO_GFX_BUFFER_SIZE * sizeof(pg_vertex_t),
        .usage = SG_USAGE_STREAM
    });

    PICO_GFX_ASSERT(sg_query_buffer_state(ctx->buffer) == SG_RESOURCESTATE_VALID);

    sg_destroy_buffer(ctx->index_buffer);

    ctx->index_buffer = sg_make_buffer(&(sg_buffer_desc)
    {
        .type  = SG_BUFFERTYPE_INDEXBUFFER,
        .size  = PICO_GFX_BUFFER_SIZE * sizeof(uint32_t),
        .usage = SG_USAGE_STREAM
    });

    PICO_GFX_ASSERT(sg_query_buffer_state(ctx->index_buffer) == SG_RESOURCESTATE_VALID);
}

void pg_push_state(pg_ctx_t* ctx)
{
    PICO_GFX_ASSERT(ctx);
    PICO_GFX_ASSERT(ctx->stack_size < PICO_GFX_STACK_MAX_SIZE);

    ctx->state_stack[ctx->stack_size] = ctx->state;
    ctx->stack_size++;
}

void pg_pop_state(pg_ctx_t* ctx)
{
    PICO_GFX_ASSERT(ctx);
    PICO_GFX_ASSERT(ctx->stack_size > 0);

    ctx->state = ctx->state_stack[ctx->stack_size - 1];
    ctx->stack_size--;

    pg_set_uniform_block(pg_get_default_shader(ctx), pg_vs_block, &ctx->state.vs_block);
}

void pg_set_clear_color(pg_ctx_t* ctx, float r, float g, float b, float a)
{
    PICO_GFX_ASSERT(ctx);
    ctx->state.clear_color = (sg_color){ r, g, b, a};
}

void pg_reset_clear_color(pg_ctx_t* ctx)
{
    PICO_GFX_ASSERT(ctx);
    pg_set_clear_color(ctx, 0.f, 0.f, 0.f, 0.f);
}

void pg_set_viewport(pg_ctx_t* ctx, int x, int y, int w, int h)
{
    PICO_GFX_ASSERT(ctx);
    ctx->state.viewport = (pg_rect_t){ x, y, w, h};
}

void pg_reset_viewport(pg_ctx_t* ctx)
{
    PICO_GFX_ASSERT(ctx);

    if (ctx->target)
    {
        const pg_texture_t* texture = ctx->target;
        pg_set_viewport(ctx, 0, 0, texture->width, texture->height);
    }
    else
    {
        pg_set_viewport(ctx, 0, 0, ctx->window_width, ctx->window_height);
    }
}

void pg_set_scissor(pg_ctx_t* ctx, int x, int y, int w, int h)
{
    PICO_GFX_ASSERT(ctx);
    ctx->state.scissor = (pg_rect_t){ x, y, w, h};
}

void pg_reset_scissor(pg_ctx_t* ctx)
{
    PICO_GFX_ASSERT(ctx);

    if (ctx->target)
    {
        const pg_texture_t* texture = ctx->target;
        pg_set_scissor(ctx, 0, 0, texture->width, texture->height);
    }
    else
    {
        pg_set_scissor(ctx, 0, 0, ctx->window_width, ctx->window_height);
    }
}

void pg_set_pipeline(pg_ctx_t* ctx, pg_pipeline_t* pipeline)
{
    PICO_GFX_ASSERT(ctx);
    PICO_GFX_ASSERT(pipeline);
    ctx->state.pipeline = pipeline;
}

void pg_reset_pipeline(pg_ctx_t* ctx)
{
    PICO_GFX_ASSERT(ctx);
    pg_set_pipeline(ctx, ctx->default_pipeline);
}

void pg_set_projection(pg_ctx_t* ctx, pg_mat4_t matrix)
{
    memcpy(ctx->state.vs_block.u_proj, matrix, sizeof(pg_mat4_t));
    pg_set_uniform_block(pg_get_default_shader(ctx), pg_vs_block, &ctx->state.vs_block);
}

void pg_reset_projection(pg_ctx_t* ctx)
{
    pg_set_projection(ctx, (pg_mat4_t)
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    });
}

void pg_set_transform(pg_ctx_t* ctx, pg_mat4_t matrix)
{
    memcpy(ctx->state.vs_block.u_tr, matrix, sizeof(pg_mat4_t));
    pg_set_uniform_block(pg_get_default_shader(ctx), pg_vs_block, &ctx->state.vs_block);
}

void pg_reset_transform(pg_ctx_t* ctx)
{
    pg_set_transform(ctx, (pg_mat4_t)
    {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    });
}

void pg_reset_state(pg_ctx_t* ctx)
{
    PICO_GFX_ASSERT(ctx);

    memset(&ctx->state, 0, sizeof(pg_state_t));

    pg_reset_clear_color(ctx);
    pg_reset_pipeline(ctx);
    pg_reset_viewport(ctx);
    pg_reset_scissor(ctx);
    pg_reset_projection(ctx);
    pg_reset_transform(ctx);
}

pg_pipeline_t* pg_create_pipeline(const pg_ctx_t* ctx,
                                  pg_shader_t* shader,
                                  const pg_pipeline_opts_t* opts)
{
    (void)ctx;

    PICO_GFX_ASSERT(shader);

    if (opts == NULL)
        opts = &(pg_pipeline_opts_t){ 0 };

    sg_pipeline_desc desc;

    memset(&desc, 0, sizeof(sg_pipeline_desc));

    desc.layout.attrs[0] = (sg_vertex_attr_state)
    {
        .format = SG_VERTEXFORMAT_FLOAT3,
        .offset = offsetof(pg_vertex_t, pos)
    };

    desc.layout.attrs[1] = (sg_vertex_attr_state)
    {
        .format = SG_VERTEXFORMAT_FLOAT4,
        .offset = offsetof(pg_vertex_t, color)
    };

    desc.layout.attrs[2] = (sg_vertex_attr_state)
    {
        .format = SG_VERTEXFORMAT_FLOAT2,
        .offset = offsetof(pg_vertex_t, uv)
    };

    desc.primitive_type = pg_map_primitive(opts->primitive);

    if (opts->blend_enabled)
    {
        const pg_blend_mode_t* blend_mode = &opts->blend;
        desc.colors[0].blend.enabled = true;
        desc.colors[0].blend.src_factor_rgb = pg_map_blend_factor(blend_mode->color_src);
        desc.colors[0].blend.dst_factor_rgb = pg_map_blend_factor(blend_mode->color_dst);
        desc.colors[0].blend.src_factor_alpha = pg_map_blend_factor(blend_mode->alpha_src);
        desc.colors[0].blend.dst_factor_alpha = pg_map_blend_factor(blend_mode->alpha_dst);
        desc.colors[0].blend.op_rgb = pg_map_blend_eq(blend_mode->color_eq);
        desc.colors[0].blend.op_alpha = pg_map_blend_eq(blend_mode->alpha_eq);
    }

    desc.colors[0].pixel_format = SG_PIXELFORMAT_RGBA8;

    if (opts->indexed)
        desc.index_type = SG_INDEXTYPE_UINT32;
    else
        desc.index_type = SG_INDEXTYPE_NONE;

    if (opts->target)
    {
        desc.depth.pixel_format = SG_PIXELFORMAT_DEPTH;
        desc.depth.write_enabled = true;
    }

    desc.shader = shader->handle;

    pg_pipeline_t* pipeline = (pg_pipeline_t*)PICO_GFX_MALLOC(sizeof(pg_pipeline_t), ctx->mem_ctx);

    pipeline->handle = sg_make_pipeline(&desc);

    PICO_GFX_ASSERT(sg_query_pipeline_state(pipeline->handle) == SG_RESOURCESTATE_VALID);

    pipeline->indexed = opts->indexed;
    pipeline->shader = shader;

    return pipeline;
}

void pg_destroy_pipeline(const pg_ctx_t* ctx, pg_pipeline_t* pipeline)
{
    (void)ctx;

    sg_destroy_pipeline(pipeline->handle);
    PICO_GFX_FREE(pipeline, ctx->mem_ctx);
}

pg_shader_t* pg_create_shader_internal(const pg_ctx_t* ctx, pg_shader_internal_t internal)
{
    (void)ctx;

    pg_shader_t* shader = (pg_shader_t*)PICO_GFX_MALLOC(sizeof(pg_shader_t), ctx->mem_ctx);

    shader->internal = internal;

    shader->desc = internal.get_shader_desc(sg_query_backend());

    PICO_GFX_ASSERT(shader->desc);
    PICO_GFX_ASSERT(pg_str_equal(shader->desc->attrs[0].name, "a_pos"));
    PICO_GFX_ASSERT(pg_str_equal(shader->desc->attrs[1].name, "a_color"));
    PICO_GFX_ASSERT(pg_str_equal(shader->desc->attrs[2].name, "a_uv"));

    shader->handle = sg_make_shader(shader->desc);

    shader->uniform_blocks = pg_hashtable_new(PG_GFX_HT_MIN_CAPACITY,
                                              PG_GFX_HT_KEY_SIZE,
                                              sizeof(pg_uniform_block_t),
                                              ctx->mem_ctx);

    shader->arena = pg_arena_new(PICO_GFX_MIN_ARENA_CAPACITY, ctx->mem_ctx);

    return shader;
}

void pg_destroy_shader(const pg_ctx_t* ctx, pg_shader_t* shader)
{
    (void)ctx;

    PICO_GFX_ASSERT(shader);
    sg_destroy_shader(shader->handle);
    pg_hashtable_free(shader->uniform_blocks);
    pg_arena_free(shader->arena);
    PICO_GFX_FREE(shader, ctx->mem_ctx);
}

pg_shader_t* pg_get_default_shader(const pg_ctx_t* ctx)
{
    PICO_GFX_ASSERT(ctx);
    return ctx->default_shader;
}

pg_pipeline_t* pg_get_default_pipeline(const pg_ctx_t* ctx)
{
    PICO_GFX_ASSERT(ctx);
    return ctx->default_pipeline;
}

uint32_t pg_get_shader_id(const pg_shader_t* shader)
{
    PICO_GFX_ASSERT(shader);
    return shader->handle.id;
}

void pg_register_uniform_block_internal(pg_shader_t* shader,
                                        pg_stage_t stage,
                                        const char* name,
                                        size_t size)
{
    PICO_GFX_ASSERT(shader);
    PICO_GFX_ASSERT(name);
    PICO_GFX_ASSERT(size > 0);

    pg_uniform_block_t block =
    {
        .slot  = shader->internal.get_uniformblock_slot(pg_map_stage(stage), name),
        .stage = stage,
        .data  = pg_arena_alloc(shader->arena, size),
        .size  = size,
        .dirty = false
    };

    pg_hashtable_put(shader->uniform_blocks, name, &block);
}

void pg_set_uniform_block_internal(pg_shader_t* shader,
                                   const char* name,
                                   const void* data)
{
    PICO_GFX_ASSERT(shader);
    PICO_GFX_ASSERT(name);
    PICO_GFX_ASSERT(data);

    pg_uniform_block_t* block = pg_hashtable_get(shader->uniform_blocks, name);

    PICO_GFX_ASSERT(block);

    memcpy(block->data, data, block->size);
    block->dirty = true;
}

pg_texture_t* pg_create_texture(const pg_ctx_t* ctx,
                                int width, int height,
                                const uint8_t* data, size_t size,
                                const pg_texture_opts_t* opts)
{
    (void)ctx;

    PICO_GFX_ASSERT(width > 0);
    PICO_GFX_ASSERT(height > 0);
    PICO_GFX_ASSERT(data);
    PICO_GFX_ASSERT(size > 0);

    if (opts == NULL)
        opts = &(pg_texture_opts_t){ 0 };

    PICO_GFX_ASSERT(opts->mipmaps >= 0);

    pg_texture_t* texture = (pg_texture_t*)PICO_GFX_MALLOC(sizeof(pg_texture_t), ctx->mem_ctx);

    sg_image_desc desc;

    memset(&desc, 0, sizeof(sg_image_desc));

    desc.pixel_format = SG_PIXELFORMAT_RGBA8;

    desc.width  = texture->width  = width;
    desc.height = texture->height = height;

    desc.num_mipmaps = opts->mipmaps;
    desc.data.subimage[0][0] = (sg_range){ .ptr = data, .size = size };

    texture->target = false;
    texture->handle = sg_make_image(&desc);

    PICO_GFX_ASSERT(sg_query_image_state(texture->handle) == SG_RESOURCESTATE_VALID);

    return texture;
}

pg_texture_t* pg_create_render_texture(const pg_ctx_t* ctx,
                                       int width, int height,
                                       const pg_texture_opts_t* opts)
{
    (void)ctx;

    PICO_GFX_ASSERT(width > 0);
    PICO_GFX_ASSERT(height > 0);

    if (opts == NULL)
        opts = &(pg_texture_opts_t){ 0 };

    PICO_GFX_ASSERT(opts->mipmaps >= 0);

    pg_texture_t* texture = (pg_texture_t*)PICO_GFX_MALLOC(sizeof(pg_texture_t), ctx->mem_ctx);

    sg_image_desc desc;

    memset(&desc, 0, sizeof(sg_image_desc));

    desc.render_target = true;
    desc.pixel_format = SG_PIXELFORMAT_RGBA8;

    desc.width  = texture->width  = width;
    desc.height = texture->height = height;

    desc.num_mipmaps = opts->mipmaps;

    texture->handle = sg_make_image(&desc);
    texture->target = true;

    PICO_GFX_ASSERT(sg_query_image_state(texture->handle) == SG_RESOURCESTATE_VALID);

    desc.pixel_format = SG_PIXELFORMAT_DEPTH;
    texture->depth_handle = sg_make_image(&desc);

    PICO_GFX_ASSERT(sg_query_image_state(texture->depth_handle) == SG_RESOURCESTATE_VALID);

    texture->attachments = sg_make_attachments(&(sg_attachments_desc)
    {
        .colors[0].image = texture->handle,
        .depth_stencil.image = texture->depth_handle,
    });

    return texture;
}

void pg_destroy_texture(const pg_ctx_t* ctx, pg_texture_t* texture)
{
    (void)ctx;

    if (texture->target)
    {
        sg_destroy_image(texture->depth_handle);
    }

    sg_destroy_image(texture->handle);
    PICO_GFX_FREE(texture, ctx->mem_ctx);
}

void pg_bind_texture(pg_ctx_t* ctx, int slot, pg_texture_t* texture)
{
    PICO_GFX_ASSERT(slot < PICO_GFX_MAX_TEXTURE_SLOTS);
    ctx->state.textures[slot] = texture;
}

void pg_reset_textures(pg_ctx_t* ctx)
{
    memset(&ctx->state.textures, 0, sizeof(ctx->state.textures));
}

uint32_t pg_get_texture_id(const pg_texture_t* texture)
{
    PICO_GFX_ASSERT(texture);
    return texture->handle.id;
}

void pg_get_texture_size(const pg_texture_t* texture, int* width, int* height)
{
    PICO_GFX_ASSERT(texture);

    if (width)
        *width = texture->width;

    if (height)
        *height = texture->height;
}

pg_sampler_t* pg_create_sampler(const pg_ctx_t* ctx,
                                const pg_sampler_opts_t* opts)
{
    (void)ctx;

    if (opts == NULL)
        opts = &(pg_sampler_opts_t){ 0 };

    pg_sampler_t* sampler = PICO_GFX_MALLOC(sizeof(*sampler), ctx->mem_ctx);

    sg_sampler_desc desc = { 0 };

    desc.min_filter = (opts->smooth) ? SG_FILTER_LINEAR : SG_FILTER_NEAREST;
    desc.mag_filter = (opts->smooth) ? SG_FILTER_LINEAR : SG_FILTER_NEAREST;

    desc.wrap_u = (opts->repeat) ? SG_WRAP_REPEAT : SG_WRAP_CLAMP_TO_EDGE;
    desc.wrap_v = (opts->repeat) ? SG_WRAP_REPEAT : SG_WRAP_CLAMP_TO_EDGE;

    sampler->handle = sg_make_sampler(&desc);

    return sampler;
}

void pg_destroy_sampler(const pg_ctx_t* ctx, pg_sampler_t* sampler)
{
    (void)ctx;

    sg_destroy_sampler(sampler->handle);
    PICO_GFX_FREE(sampler, ctx->mem_ctx);
}

void pg_bind_sampler(pg_ctx_t* ctx, int slot, pg_sampler_t* sampler)
{
    ctx->state.samplers[slot] = sampler;
}

void pg_reset_samplers(pg_ctx_t* ctx)
{
    memset(&ctx->state.samplers, 0, sizeof(ctx->state.samplers));
}

static void pg_apply_uniforms(pg_shader_t* shader)
{
    PICO_GFX_ASSERT(shader);

    pg_hashtable_iterator_t iterator;
    pg_hashtable_init_iterator(shader->uniform_blocks, &iterator);

    char* key = NULL;
    void* value = NULL;

    while (pg_hashtable_iterator_next(&iterator, &key, &value))
    {
        pg_uniform_block_t* block = (pg_uniform_block_t*)value;

        //if (block->dirty)
        //{
            sg_range range = { .ptr = block->data, .size = block->size };

            sg_shader_stage stage = pg_map_stage(block->stage);

            sg_apply_uniforms(stage, block->slot, &range);

            block->dirty = false;
        //}
    }
}

pg_vbuffer_t* pg_create_vbuffer(const pg_ctx_t* ctx,
                                const pg_vertex_t* vertices,
                                size_t count)
{
    (void)ctx;

    PICO_GFX_ASSERT(vertices);
    PICO_GFX_ASSERT(count > 0);

    pg_vbuffer_t* buffer = (pg_vbuffer_t*)PICO_GFX_MALLOC(sizeof(pg_vbuffer_t), ctx->mem_ctx);

    buffer->handle = sg_make_buffer(&(sg_buffer_desc)
    {
        .type  = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = SG_USAGE_IMMUTABLE,
        .data  = { .ptr = vertices, .size = count * sizeof(pg_vertex_t) }
    });

    PICO_GFX_ASSERT(sg_query_buffer_state(buffer->handle) == SG_RESOURCESTATE_VALID);

    buffer->count = count;

    return buffer;
}

void pg_destroy_vbuffer(const pg_ctx_t* ctx, pg_vbuffer_t* buffer)
{
    (void)ctx;

    PICO_GFX_ASSERT(buffer);
    sg_destroy_buffer(buffer->handle);
    PICO_GFX_FREE(buffer, ctx->mem_ctx);
}

static void pg_apply_view_state(const pg_ctx_t* ctx)
{
    const pg_rect_t* vp_rect = &ctx->state.viewport;
    sg_apply_viewport(vp_rect->x, vp_rect->y, vp_rect->width, vp_rect->height, true);

    const pg_rect_t* s_rect = &ctx->state.scissor;
    sg_apply_scissor_rect(s_rect->x, s_rect->y, s_rect->width, s_rect->height, true);
}

static void pg_apply_textures(const pg_ctx_t* ctx, sg_bindings* bindings)
{
    for (int i = 0; i < PICO_GFX_MAX_TEXTURE_SLOTS; i++)
    {
        if (!ctx->state.textures[i])
            continue;

        bindings->fs.images[i] = ctx->state.textures[i]->handle;
    }
}

static void pg_apply_samplers(const pg_ctx_t* ctx, sg_bindings* bindings)
{
    for (int i = 0; i < PICO_GFX_MAX_SAMPLER_SLOTS; i++)
    {
        if (!ctx->state.samplers[i])
            continue;

        bindings->fs.samplers[i] = ctx->state.samplers[i]->handle;
    }
}

void pg_draw_vbuffer(const pg_ctx_t* ctx,
                     const pg_vbuffer_t* buffer,
                     size_t start, size_t count)
{
    PICO_GFX_ASSERT(ctx);
    PICO_GFX_ASSERT(buffer);
    PICO_GFX_ASSERT(ctx->pass_active);
    PICO_GFX_ASSERT(!ctx->state.pipeline->indexed);

    sg_bindings bindings;

    memset(&bindings, 0, sizeof(sg_bindings));

    pg_apply_textures(ctx, &bindings);
    pg_apply_samplers(ctx, &bindings);

    bindings.vertex_buffers[0] = buffer->handle;

    pg_apply_view_state(ctx);

    pg_pipeline_t* pipeline = ctx->state.pipeline;

    sg_apply_pipeline(pipeline->handle);
    sg_apply_bindings(&bindings);
    pg_apply_uniforms(pipeline->shader);

    PICO_GFX_ASSERT(start + count <= buffer->count);

    sg_draw(start, count, 1);
}

void pg_draw_array(pg_ctx_t* ctx, const pg_vertex_t* vertices, size_t count)
{
    PICO_GFX_ASSERT(ctx);
    PICO_GFX_ASSERT(vertices);
    PICO_GFX_ASSERT(count > 0);
    PICO_GFX_ASSERT(ctx->pass_active);
    PICO_GFX_ASSERT(!ctx->state.pipeline->indexed);

    int offset = sg_append_buffer(ctx->buffer, &(sg_range)
    {
        .ptr = vertices,
        .size = count * sizeof(pg_vertex_t)
    });

    sg_bindings bindings = { 0 };

    pg_apply_textures(ctx, &bindings);
    pg_apply_samplers(ctx, &bindings);

    bindings.vertex_buffer_offsets[0] = offset;
    bindings.vertex_buffers[0] = ctx->buffer;

    pg_apply_view_state(ctx);

    pg_pipeline_t* pipeline = ctx->state.pipeline;

    sg_apply_pipeline(pipeline->handle);
    sg_apply_bindings(&bindings);
    pg_apply_uniforms(pipeline->shader);

    sg_draw(0, count, 1);
}

void pg_draw_indexed_array(pg_ctx_t* ctx,
                           const pg_vertex_t* vertices, size_t vertex_count,
                           const uint32_t* indices, size_t index_count)
{
    PICO_GFX_ASSERT(ctx);
    PICO_GFX_ASSERT(vertices);
    PICO_GFX_ASSERT(vertex_count > 0);
    PICO_GFX_ASSERT(indices);
    PICO_GFX_ASSERT(index_count > 0);
    PICO_GFX_ASSERT(ctx->pass_active);
    PICO_GFX_ASSERT(ctx->state.pipeline->indexed);

    int vertex_offset = sg_append_buffer(ctx->buffer, &(sg_range)
    {
        .ptr = vertices,
        .size = vertex_count * sizeof(pg_vertex_t)
    });

    int index_offset = sg_append_buffer(ctx->index_buffer, &(sg_range)
    {
        .ptr = indices,
        .size = index_count * sizeof(uint32_t)
    });

    sg_bindings bindings = { 0 };

    pg_apply_textures(ctx, &bindings);
    pg_apply_samplers(ctx, &bindings);

    bindings.vertex_buffer_offsets[0] = vertex_offset;
    bindings.index_buffer_offset = index_offset;
    bindings.vertex_buffers[0] = ctx->buffer;
    bindings.index_buffer = ctx->index_buffer;

    pg_apply_view_state(ctx);

    pg_pipeline_t* pipeline = ctx->state.pipeline;

    sg_apply_pipeline(pipeline->handle);
    sg_apply_bindings(&bindings);
    pg_apply_uniforms(pipeline->shader);

    sg_draw(0, index_count, 1);
}

/*==============================================================================
 * GFX Static Functions
 *============================================================================*/

static sg_primitive_type pg_map_primitive(pg_primitive_t primitive)
{
    if (primitive == PG_DEFAULT_PRIMITIVE)
        primitive = PG_TRIANGLES;

    switch (primitive)
    {
        case PG_POINTS:         return SG_PRIMITIVETYPE_POINTS;
        case PG_LINES:          return SG_PRIMITIVETYPE_LINES;
        case PG_LINE_STRIP:     return SG_PRIMITIVETYPE_LINE_STRIP;
        case PG_TRIANGLES:      return SG_PRIMITIVETYPE_TRIANGLES;
        case PG_TRIANGLE_STRIP: return SG_PRIMITIVETYPE_TRIANGLE_STRIP;
        default: PICO_GFX_ASSERT(false);
    }
}

static sg_blend_factor pg_map_blend_factor(pg_blend_factor_t factor)
{
    if (factor == PG_DEFAULT_BLEND_FACTOR)
        factor = PG_ONE;

    switch (factor)
    {
        case PG_ZERO:                return SG_BLENDFACTOR_ZERO;
        case PG_ONE:                 return SG_BLENDFACTOR_ONE;
        case PG_SRC_COLOR:           return SG_BLENDFACTOR_SRC_COLOR;
        case PG_ONE_MINUS_SRC_COLOR: return SG_BLENDFACTOR_ONE_MINUS_SRC_COLOR;
        case PG_DST_COLOR:           return SG_BLENDFACTOR_DST_COLOR;
        case PG_ONE_MINUS_DST_COLOR: return SG_BLENDFACTOR_ONE_MINUS_DST_COLOR;
        case PG_SRC_ALPHA:           return SG_BLENDFACTOR_SRC_ALPHA;
        case PG_ONE_MINUS_SRC_ALPHA: return SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        case PG_DST_ALPHA:           return SG_BLENDFACTOR_DST_ALPHA;
        case PG_ONE_MINUS_DST_ALPHA: return SG_BLENDFACTOR_ONE_MINUS_DST_ALPHA;
        default: PICO_GFX_ASSERT(false);
    }
}

static sg_blend_op pg_map_blend_eq(pg_blend_eq_t eq)
{
    if (eq == PG_DEFAULT_BLEND_EQ)
        eq = PG_ADD;

    switch (eq)
    {
        case PG_ADD:              return SG_BLENDOP_ADD;
        case PG_SUBTRACT:         return SG_BLENDOP_SUBTRACT;
        case PG_REVERSE_SUBTRACT: return SG_BLENDOP_REVERSE_SUBTRACT;
        default: PICO_GFX_ASSERT(false);
    }
}

static sg_shader_stage pg_map_stage(pg_stage_t stage)
{
    switch (stage)
    {
        case PG_VS_STAGE: return SG_SHADERSTAGE_VS;
        case PG_FS_STAGE: return SG_SHADERSTAGE_FS;
        default: PICO_GFX_ASSERT(false);
    }
}

static void pg_log(const char* fmt, ...)
{
    PICO_GFX_ASSERT(fmt);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");
    fflush(stdout);
}

static void pg_log_sg(const char* tag,                // e.g. 'sg'
                      uint32_t log_level,             // 0=panic, 1=error, 2=warn, 3=info
                      uint32_t log_item_id,           // SG_LOGITEM_*
                      const char* message_or_null,    // a message string, may be nullptr in release mode
                      uint32_t line_nr,               // line number in sokol_gfx.h
                      const char* filename_or_null,   // source filename, may be nullptr in release mode
                      void* user_data)
{
    (void)log_item_id;
    (void)user_data;

    static const char* level[] =
    {
        "Panic",
        "Error",
        "Warn",
        "Info",
    };

    // FIXME: Handle non-debug case

    if (message_or_null && !filename_or_null)
    {
        PICO_GFX_LOG("Tag: %s, Level: %s, Message: %s",
                      tag, level[log_level], message_or_null);
    }

    if (!message_or_null && filename_or_null)
    {
        PICO_GFX_LOG("Tag: %s, Level: %s, File: %s, Line: %d",
                      tag, level[log_level], filename_or_null, line_nr);
    }

    if (message_or_null && filename_or_null)
    {
        PICO_GFX_LOG("Tag: %s, Level: %s, File: %s, Line: %d, Message: %s",
                      tag, level[log_level], filename_or_null, line_nr, message_or_null);
    }
}

/*==============================================================================
 * Hashtable Data Structures
 *============================================================================*/

typedef struct
{
    pg_hash_t hash;
    char* key;
    void* value;
} pg_hashtable_entry_t;

struct pg_hashtable_t
{
    void* mem_ctx;
    size_t capacity;
    size_t size;

    pg_hashtable_entry_t* entries;

    size_t key_size;
    char* keys;

    size_t value_size;
    void* values;
};

/*==============================================================================
 * Hashtable Internal Declarations
 *============================================================================*/

static size_t pg_hashtable_compute_hash(const pg_hashtable_t* ht,
                                        const char* key);

static bool pg_hashtable_key_equal(const pg_hashtable_t* ht,
                                   const char* key1,
                                   const char* key2);

static void pg_hashtable_copy_value(pg_hashtable_t* ht,
                                    pg_hashtable_entry_t* entry,
                                    const void* value);

static void pg_hashtable_swap_size(size_t* a, size_t* b);
static void pg_hashtable_swap_ptr(void** a, void** b);
static void pg_hashtable_swap(pg_hashtable_t* ht1, pg_hashtable_t* ht2);
static void pg_hashtable_rehash(pg_hashtable_t* ht);

/*==============================================================================
 * Hashtable Public Implementation
 *============================================================================*/

static pg_hashtable_t* pg_hashtable_new(size_t capacity,
                                        size_t key_size,
                                        size_t value_size,
                                        void* mem_ctx)
{
    bool power_of_two = (0 == (capacity & (capacity - 1)));

    PICO_GFX_ASSERT(capacity > 2 && power_of_two);
    PICO_GFX_ASSERT(key_size > 0);
    PICO_GFX_ASSERT(value_size > 0);

    if (capacity <= 2 || !power_of_two)
        return NULL;

    if (0 == key_size || 0 == value_size)
        return NULL;

    pg_hashtable_t* ht = PICO_GFX_MALLOC(sizeof(pg_hashtable_t), mem_ctx);

    if (!ht)
        return NULL;

    ht->mem_ctx = mem_ctx;
    ht->capacity = capacity;
    ht->size = 0;
    ht->key_size = key_size;
    ht->value_size = value_size;

    ht->entries = PICO_GFX_MALLOC(capacity * sizeof(pg_hashtable_entry_t), mem_ctx);

    if (!ht->entries)
    {
        PICO_GFX_FREE(ht, mem_ctx);
        return NULL;
    }

    ht->keys = PICO_GFX_MALLOC(capacity * key_size, mem_ctx);

    if (!ht->keys)
    {
        PICO_GFX_FREE(ht->entries, mem_ctx);
        PICO_GFX_FREE(ht, mem_ctx);

        return NULL;
    }

    ht->values = PICO_GFX_MALLOC(capacity * value_size, mem_ctx);

    if (!ht->values)
    {
        PICO_GFX_FREE(ht->entries, mem_ctx);
        PICO_GFX_FREE(ht->keys, mem_ctx);
        PICO_GFX_FREE(ht, mem_ctx);
        return NULL;
    }

    for (size_t i = 0; i < capacity; i++)
    {
        pg_hashtable_entry_t* entry = &ht->entries[i];
        entry->hash = 0;
        entry->key = (char*)ht->keys + i * key_size;
        entry->value = (char*)ht->values + i * value_size;
    }

    return ht;
}

static void pg_hashtable_free(pg_hashtable_t* ht)
{
    PICO_GFX_ASSERT(NULL != ht);

    PICO_GFX_FREE(ht->entries, ht->mem_ctx);
    PICO_GFX_FREE(ht->keys, ht->mem_ctx);
    PICO_GFX_FREE(ht->values, ht->mem_ctx);
    PICO_GFX_FREE(ht, ht->mem_ctx);
}

static void pg_hashtable_init_iterator(const pg_hashtable_t* ht,
                                       pg_hashtable_iterator_t* iterator)
{
    PICO_GFX_ASSERT(NULL != ht);
    iterator->ht = ht;
    iterator->index = 0;
    iterator->count = 0;
}

static bool pg_hashtable_iterator_next(pg_hashtable_iterator_t* iterator,
                                       char** key, void** value)
{
    PICO_GFX_ASSERT(NULL != iterator);

    const pg_hashtable_t* ht = iterator->ht;

    if (iterator->count >= ht->capacity)
        return false;

    while (iterator->index < ht->capacity)
    {
        pg_hashtable_entry_t* entry = &ht->entries[iterator->index];

        if (entry->hash != 0)
        {
            if (key)
                *key = entry->key;

            if (value)
                *value = entry->value;

            iterator->count++;
            iterator->index++;

            return true;
        }

        iterator->index++;
    }

    return false;
}

static void pg_hashtable_put(pg_hashtable_t* ht,
                             const char* key,
                             const void* value)
{
    PICO_GFX_ASSERT(NULL != ht);

    if (ht->size == ht->capacity)
    {
        pg_hashtable_rehash(ht);
        PICO_GFX_ASSERT(ht->capacity > 0);
    }

    pg_hash_t hash = pg_hashtable_compute_hash(ht, key);

    PICO_GFX_ASSERT(hash > 0);

    size_t start_index = hash % ht->capacity;
    size_t index = start_index;

    do
    {
        pg_hashtable_entry_t* entry = &ht->entries[index];

        if (entry->hash == hash && pg_hashtable_key_equal(ht, key, entry->key))
        {
            pg_hashtable_copy_value(ht, entry, value);
            break;
        }

        if (entry->hash == 0)
        {
            entry->hash = hash;

            memcpy(entry->key, key, ht->key_size); //FIXME: use string copy function
            pg_hashtable_copy_value(ht, entry, value);

            ht->size++;

            break;
        }

        index = (index + 1) % ht->capacity;

    } while (index != start_index);

    start_index = index;
    index = (index + 1) % ht->capacity;

    while (index != start_index)
    {
        pg_hashtable_entry_t* entry = &ht->entries[index];

        if (entry->hash == hash && pg_hashtable_key_equal(ht, key, entry->key))
        {
            entry->hash = 0;
            ht->size--;
            return;
        }

        index = (index + 1) % ht->capacity;
    }
}

static void* pg_hashtable_get(const pg_hashtable_t* ht, const char* key)
{
    PICO_GFX_ASSERT(NULL != ht);

    pg_hash_t hash = pg_hashtable_compute_hash(ht, key);

    PICO_GFX_ASSERT(hash > 0);

    size_t start_index = hash % ht->capacity;
    size_t index = start_index;

    do
    {
        pg_hashtable_entry_t* entry = &ht->entries[index];

        if (entry->hash == hash && pg_hashtable_key_equal(ht, key, entry->key))
        {
            return entry->value;
        }

        index = (index + 1) % ht->capacity;

    } while (index != start_index);

    return NULL;
}

/*==============================================================================
 * Hashtable Internal API
 *============================================================================*/

static bool pg_hashtable_key_equal(const pg_hashtable_t* ht,
                                   const char* key1,
                                   const char* key2)
{
    return 0 == strncmp(key1, key2, ht->key_size);
}

static void pg_hashtable_copy_value(pg_hashtable_t* ht,
                                    pg_hashtable_entry_t* entry,
                                    const void* value)
{
    memcpy(entry->value, value, ht->value_size);
}

static void pg_hashtable_swap_size(size_t* a, size_t* b)
{
    size_t tmp = *a;
    *a = *b;
    *b = tmp;
}

static void pg_hashtable_swap_ptr(void** a, void** b)
{
    void* tmp = *a;
    *a = *b;
    *b = tmp;
}

static void pg_hashtable_swap(pg_hashtable_t* ht1, pg_hashtable_t* ht2)
{
    pg_hashtable_swap_size(&ht1->capacity, &ht2->capacity);
    pg_hashtable_swap_size(&ht1->size, &ht2->size);

    pg_hashtable_swap_ptr((void**)&ht1->entries, (void**)&ht2->entries);

    pg_hashtable_swap_size(&ht1->key_size, &ht2->key_size);
    pg_hashtable_swap_ptr((void**)&ht1->keys, (void**)&ht2->keys);

    pg_hashtable_swap_size(&ht1->value_size, &ht2->value_size);
    pg_hashtable_swap_ptr(&ht1->values, &ht2->values);
}

static void pg_hashtable_rehash(pg_hashtable_t* ht)
{
    pg_hashtable_t* new_ht = pg_hashtable_new(ht->capacity * 2,
                                              ht->key_size,
                                              ht->size,
                                              ht->mem_ctx);

    pg_hashtable_iterator_t iterator;
    pg_hashtable_init_iterator(ht, &iterator);

    char* key;
    void* value;

    while (pg_hashtable_iterator_next(&iterator, &key, &value))
    {
        pg_hashtable_put(new_ht, key, value);
    }

    pg_hashtable_swap(ht, new_ht);

    pg_hashtable_free(new_ht);
}

/*==============================================================================
 * Hash Functions
 *============================================================================*/

static size_t pg_hashtable_compute_hash(const pg_hashtable_t* ht, const char* key)
{

#ifdef PICO_GFX_32BIT
    static const uint32_t offset_basis = 0x811C9DC5;
    static const uint32_t prime = 0x1000193;
#else
    static const uint64_t offset_basis = 0xCBF29CE484222325;
    static const uint64_t prime = 0x100000001B3;
#endif

    const char* data = key;

    pg_hash_t hash = offset_basis;

    for (size_t i = 0; i < ht->key_size; i++) {
        hash ^= (pg_hash_t)data[i];
        hash *= prime;
    }

    // Ensure hash is never zero
    if (hash == 0)
        hash++;

    return hash;
}

/*==============================================================================
 * Arena Allocator
 *============================================================================*/

struct pg_arena_t
{
    void*  mem_ctx;
    size_t capacity;
    size_t size;
    void*  block;
};

static pg_arena_t* pg_arena_new(size_t size, void* mem_ctx)
{
    PICO_GFX_ASSERT(size > 0);

    pg_arena_t* arena = PICO_GFX_MALLOC(sizeof(pg_arena_t), mem_ctx);

    memset(arena, 0, sizeof(pg_arena_t));

    arena->mem_ctx = mem_ctx;
    arena->capacity = size * 2;
    arena->block = PICO_GFX_MALLOC(arena->capacity, mem_ctx);
    arena->size = size;

    return arena;
}

static void* pg_arena_alloc(pg_arena_t* arena, size_t size)
{
    if (arena->size + size > arena->capacity)
    {
        while (arena->size + size >= arena->capacity)
        {
            arena->capacity *= 2;
        }

        arena->block = PICO_GFX_REALLOC(arena->block, arena->capacity, arena->mem_ctx);
    }

    void* mem = (char*)arena->block + arena->size;

    arena->size += size;

    return mem;
}

static void pg_arena_free(pg_arena_t* arena)
{
    PICO_GFX_FREE(arena->block, arena->mem_ctx);
    PICO_GFX_FREE(arena, arena->mem_ctx);
}

#define SOKOL_GFX_IMPL
#include "sokol_gfx.h"

#define SOKOL_SHDC_IMPL
#include "pico_gfx_shader.h"

#endif //PICO_GFX_IMPLEMENTATION

/*
    ----------------------------------------------------------------------------
    This software is available under two licenses (A) or (B). You may choose
    either one as you wish:
    ----------------------------------------------------------------------------

    (A) The zlib License

    Copyright (c) 2023 James McLean

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
