/**
    @file pico_gfx.h
    @brief A powerful graphics library based on Sokol GFX, written in C99.

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Features:
    ---------

    - Written in C99
    - Two header library for easy build system integration
    - Easy to use low-level constructs (buffers, render passes, pipelines, and samplers)
    - Flexible pipeline configuration
    - Simple state management system (state stack)
    - Render to texture
    - Custom shaders via the sokol shader compiler
    - Simple and concise API
    - Permissive license (zlib or public domain)

    Summary:
    --------

    pico_gfx is a thin wrapper for [sokol_gfx](https://github.com/floooh/sokol/blob/master/sokol_gfx.h),
    a low-level graphics library that supports OpenGL, Metal, D3D, and WebGPU.
    pico_gfx is designed make the common case intuitive and convenient. It
    provides access to low-level constructs, such as buffers, render passes and
    pipelines, in a way that is easy to use and understand.

    pico_gfx comes with three examples; basic quad rendering (to a render
    texture and the screen), a scene graph demo, and a particle system demo.
    These are the best source of information regadring how to use the API.

    In constrast with earlier versions, pico_gfx no longer includes a default
    shader or pipeline. This was a hard decision to make, but the header is less
    cluttered, and more generic. It is also hard to define what default really
    means. Pipeline layouts must now be specified explicitly. The examples
    demonstrate several ways of doing this.

    Shaders must be compiled with the sokol compiler (`sokol-shdc`). Binary
    versions of which can be found [here](https://github.com/floooh/sokol-tools-bin).
    The source code for the compiler can be found [here](https://github.com/floooh/sokol-tools).
    An example of how to use the compiler can be found in the
    `build_pico_gfx_shader` directory. There are also two compiled shaders
    included with the examples that are more or less generic for rendering
    sprites and particles.

    One thing pico_gfx does not support (and neither does sokol_gfx) is window
    and graphics context creation. See [here](https://github.com/RandyGaul/cute_framework/tree/master/src/internal)
    for some examples. It is worth mentioning that [SDL2](https://www.libsdl.org)
    can supply both a window and OpenGL context out of the box. SDL2 is used
    in the demos.

    Another library that supports window/context creation on all supported
    backends is [sokol_app](https://github.com/floooh/sokol/blob/master/sokol_app.h),
    but it has yet to be tested with pico_gfx.

    The state that pico_gfx manages includes:

        - Pipelines/shaders
        - Uniform blocks
        - Vertex buffers
        - Index buffers
        - Clear color
        - Viewport
        - Scissor
        - Textures
        - Samplers

    Most changes to the state can be isolated by using the state stack
    (`pg_push_state/pg_pop_state`). Simply push the current state onto the
    stack, make some local changes, and then pop the stack to restore the
    original state. The exceptions are textures and samplers that are shader
    state and not global state.

    Shaders expose uniforms in blocks. These blocks must be registered with the
    shader by calling `pg_alloc_uniform_block`. They may then be set at will
    by calling `pg_set_uniform_block`. These functions operate on structs
    supplied by a compiled shader,

    Please see the examples for more details.

    C++
    --------

    In this iteration, pico_gfx is a C only library. Proper C++ compatibility
    may be introduced in the future.

    Build:
    --------

    To use this library in your project, add

    > #define PICO_GFX_IMPLEMENTATION
    > #include "pico_gfx.h"

    to a source file.

    IMPORTANT: sokol_gfx.h must be in the include path!

    You must also define one of

    #define PICO_GFX_GL
    #define PICO_GFX_GLES
    #define PICO_GFX_D3D
    #define PICO_GFX_METAL
    #define PICO_GFX_WEBGPU

    before including pico_gfx.h

    See the examples for build details.

    Constants:
    --------

    - PICO_GFX_HASHTABLE_KEY_SIZE (default: 16)
    - PICO_GFX_STACK_MAX_SIZE (default: 16)

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

#ifndef PICO_GFX_H
#define PICO_GFX_H

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

#include "sokol_gfx.h"

#include <stdbool.h>
#include <stddef.h>

#define PG_MAX_VERTEX_ATTRIBUTES SG_MAX_VERTEX_ATTRIBUTES
#define PG_MAX_VERTEX_BUFFERS    SG_MAX_VERTEX_BUFFERS
#define PG_MAX_TEXTURE_SLOTS     SG_MAX_SHADERSTAGE_IMAGES
#define PG_MAX_SAMPLER_SLOTS     SG_MAX_SHADERSTAGE_SAMPLERS

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
 * @brief Shader stage
*/
typedef enum
{
    PG_STAGE_VS, //!< Vertex shader stage
    PG_STAGE_FS  //!< Fragment shader stage
} pg_stage_t;

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
 * @brief A vertex or index array buffer
 */
typedef struct pg_buffer_t pg_buffer_t;

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
pg_backend_t pg_backend(void);

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
 * MVP transform, buffers, textures, and samplers
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
 * @param pipeline The pipeline to be activated
 */
void pg_set_pipeline(pg_ctx_t* ctx, pg_pipeline_t* pipeline);

/**
 * Resets the pipeline
 */
void pg_reset_pipeline(pg_ctx_t* ctx);

/**
 * @brief Binds a buffer to the specified slot
 */
void pg_bind_buffer(pg_ctx_t* ctx, int slot, pg_buffer_t* buffer);

/**
 * @brief Clears buffer bindings
 */
void pg_reset_buffers(pg_ctx_t* ctx);

/**
 * @brief Sets the active index buffer
 *
 * If an index buffer is set, pg_draw will use indexing.
 */
void pg_set_index_buffer(pg_ctx_t* ctx, pg_buffer_t* buffer);

/**
 *  @brief Disables indexed rednering
 */
void pg_reset_index_buffer(pg_ctx_t* ctx);

/**
 * @brief Binds a texture to a slot in the current state
 * @param shader The shader associated with the texture
 * @param slot The binding slot
 * @param texture The texture to bind
 */
void pg_bind_texture(pg_shader_t* shader, const char* name, pg_texture_t* texture);

/**
 * @brief Resets the texture bindings for the current state
 */
void pg_reset_textures(pg_shader_t* shader);

/**
 * @brief Binds a sampler to a slot in the current state
 * @param shader The shader associated with the sampler
 * @param slot The binding slot
 * @param sampler The sampler to bind
 */
void pg_bind_sampler(pg_shader_t* shader, const char* name, pg_sampler_t* sampler);

/**
 * @brief Resets the sampler bindings for the current state
 */
void pg_reset_samplers(pg_shader_t* shader);

/**
 * @brief Resets the active state to defaults
 */
void pg_reset_state(pg_ctx_t* ctx);

/**
 * @brief Creates the shader with the given prefix
 * The prefix should refer to the shader program name in a shader compiled by
 * `sokol-shdc`. For example the sprite shader in the examples would have the
 * prefix 'shader' (without quotation marks)
 */
#define pg_create_shader(ctx, prefix)   \
    pg_create_shader_internal(          \
        ctx,                            \
        (pg_shader_internal_t)          \
        {                               \
            prefix##_shader_desc,       \
            prefix##_attr_slot,         \
            prefix##_image_slot,        \
            prefix##_sampler_slot,      \
            prefix##_uniformblock_slot, \
            prefix##_uniformblock_size, \
        }                               \
    )

/**
 * @brief Destroys a shader
 */
void pg_destroy_shader(pg_shader_t* shader);

/**
 * @brief Returns a shader ID
 */
uint32_t pg_get_shader_id(const pg_shader_t* shader);

/**
 * @brief Registers a uniform block (UB)
 * @param shader The shader owning the UB
 * @param stage The stage (VS or FS) associated with the UB
 * @param name The name of the UB as supplied by `sokol_shdc`
 */
void pg_alloc_uniform_block(pg_shader_t* shader, pg_stage_t stage, const char* name);

/**
 * @brief Sets a uniform block (UB)
 * @param shader The shader owning the UB
 * @param name The name of the UB as supplied by `sokol_shdc`
 * @param data The data to set (must be the whole UB)
 */
void pg_set_uniform_block(pg_shader_t* shader, const char* name, const void* data);

/**
 * @brief Vertex attribute pixel formats
 */
typedef enum
{
    PG_VERTEX_FORMAT_INVALID,
    PG_VERTEX_FORMAT_FLOAT,
    PG_VERTEX_FORMAT_FLOAT2,
    PG_VERTEX_FORMAT_FLOAT3,
    PG_VERTEX_FORMAT_FLOAT4,
    PG_VERTEX_FORMAT_BYTE4,
    PG_VERTEX_FORMAT_BYTE4N,
    PG_VERTEX_FORMAT_UBYTE4,
    PG_VERTEX_FORMAT_UBYTE4N,
    PG_VERTEX_FORMAT_SHORT2,
    PG_VERTEX_FORMAT_SHORT2N,
    PG_VERTEX_FORMAT_USHORT2N,
    PG_VERTEX_FORMAT_SHORT4,
    PG_VERTEX_FORMAT_SHORT4N,
    PG_VERTEX_FORMAT_USHORT4N,
    PG_VERTEX_FORMAT_UINT10_N2,
    PG_VERTEX_FORMAT_HALF2,
    PG_VERTEX_FORMAT_HALF4,
} pg_vertex_format_t;

/**
 * @brief Vertex buffer description
 */
typedef struct
{
    bool instanced; //!< True if the buffer will be used for instanced rendering
    int step;       //!< The step rate (default is 1)
} pg_vertex_buf_t;

/**
 * @brief Vertex attribute description
 */
typedef struct
{
    int buffer_index;          //!< The vertex buffer bind slot
    pg_vertex_format_t format; //!< Vertex pixel format (see above)
    int offset;                //!< Attribute offset into the vertex buffer
} pg_vertex_attr_t;

/**
 * @brief Pipeline layout
 */
typedef struct
{
    pg_vertex_buf_t  bufs[PG_MAX_VERTEX_BUFFERS];     //!< Vertex buffer descriptions
    pg_vertex_attr_t attrs[PG_MAX_VERTEX_ATTRIBUTES]; //!< Vertex buffer attribute definitions
} pg_pipeline_layout_t;

/**
 * @brief Pipeline creation options
 */
typedef struct pg_pipeline_opts_t
{
    pg_primitive_t primitive;    //!< Rendering primitive
    pg_pipeline_layout_t layout; //!< Attribute information
    bool target;                 //!< Drawing to render target
    bool indexed;                //!< Indexed drawing
    bool blend_enabled;          //!< Enables blending
    pg_blend_mode_t blend;       //!< Blend mode
} pg_pipeline_opts_t;

/**
 * @brief Creates a rendering pipeline (encapsulates render state)
 * @param shader The shader used by this pipeline
 * @param opts Pipeline creation options (required!)
 * @returns A render pipeline object
 */
pg_pipeline_t* pg_create_pipeline(pg_ctx_t* ctx,
                                  pg_shader_t* shader,
                                  const pg_pipeline_opts_t* opts);

/**
 * @brief Destroys a render pipeline
*/
void pg_destroy_pipeline(pg_pipeline_t* pipeline);

/**
 * @brief Returns the shader associated with the pipeline
 */
pg_shader_t* pg_get_pipeline_shader(const pg_pipeline_t* pipeline);

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
pg_texture_t* pg_create_texture(pg_ctx_t* ctx,
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
pg_texture_t* pg_create_render_texture(pg_ctx_t* ctx,
                                       int width, int height,
                                       const pg_texture_opts_t* opts);

/**
 * @brief Destroys a texture
 */
void pg_destroy_texture(pg_texture_t* texture);

/**
 * @brief Returns a texture ID
 */
uint32_t pg_get_texture_id(const pg_texture_t* texture);

/**
 * @brief Gets a texture's dimensions
 */
void pg_get_texture_size(const pg_texture_t* texture, int* width, int* height);

/**
 * @brief Updates a texture with the given data. This can only be called once
 * per frame
 */
void pg_update_texture(pg_texture_t* texture, char* data, int width, int height);

/**
 * @brief Sampler options
 */
typedef struct
{
    bool smooth;   //!< Linear filtering if true, nearest otherwise
    bool repeat_u; //!< Repeat if true, clamp-to-edge otherwise
    bool repeat_v; //!< Repeat if true, clamp-to-edge otherwise
} pg_sampler_opts_t;

/**
 * @brief Creates a sampler represents an object that can control how shaders
 * transform and filter texture resource data.
 * @param opts Sampler options
 */
pg_sampler_t* pg_create_sampler(pg_ctx_t* ctx, const pg_sampler_opts_t* opts);

/**
 * @brief Destroys a sampler object
 */
void pg_destroy_sampler(pg_sampler_t* sampler);

/**
 *  @brief Buffer update criteria
 */
typedef enum
{
    PG_USAGE_STATIC,  //!< Buffer is immutable (cannot be updated)
    PG_USAGE_DYNAMIC, //!< Buffer is updated on average less than once per frame
    PG_USAGE_STREAM   //!< Buffer is updated possibly more than once per frame
} pg_buffer_usage_t;

/**
 * @brief Creates a vertex buffer
 * @param usage Determines whether the buffer is static, dynamic, or streaming
 * @param data Vertex data elements (can be NULL)
 * @param count The number of elements (can be zero)
 * @param max_elements The maximum number of elements in the buffer
 * @param element_size The size (in bytes) of each individual element
 */
pg_buffer_t* pg_create_vertex_buffer(pg_ctx_t* ctx,
                                     pg_buffer_usage_t usage,
                                     const void* data,
                                     size_t count,
                                     size_t max_elements,
                                     size_t element_size);

/**
 * @brief Creates a vertex buffer
 * @param usage Determines whether the buffer is static, dynamic, or streaming
 * @param data Index data (can be NULL)
 * @param count The number of indices (can be zero)
 * @param max_elements The maximum number of indices in the buffer
 */
pg_buffer_t* pg_create_index_buffer(pg_ctx_t* ctx,
                                    pg_buffer_usage_t usage,
                                    const void* data,
                                    size_t count,
                                    size_t max_elements);

/**
 * @brief Destroys a vertex or index buffer
 */
void pg_destroy_buffer(pg_buffer_t* buffer);

/**
 * Replaces the data in a buffer. This may only happen once per frame and cannot
 * happen after appending data
 */
void pg_update_buffer(pg_buffer_t* buffer, void* data, size_t count);

/**
 * @brief Appends data to a buffer. This can happen more than once per frame,
 * and cannot happen after an update.
 */
int pg_append_buffer(pg_buffer_t* buffer, void* data, size_t count);

/**
 * @brief Returns the buffer offset
 */
int pg_get_buffer_offset(pg_buffer_t* buffer);

/**
 * @brief Sets the buffer offset
 */
void pg_set_buffer_offset(pg_buffer_t* buffer, int offset);

/**
 * @brief Destroys and recreates buffer
 */
void pg_reset_buffer(pg_buffer_t* buffer);

/**
 * @brief Draws from the buffers that are bound to the current state
 * @param ctx The graphics context
 * @param start The position of the first element
 * @param count The number of elements to draw
 * @param instances The number of instances
 */
void pg_draw(const pg_ctx_t* ctx, size_t start, size_t count, size_t instances);

/*=============================================================================
 * Internals
 *============================================================================*/

typedef struct
{
	const sg_shader_desc* (*get_shader_desc)(sg_backend backend);
	int (*get_attr_slot)(const char* attr_name);
	int (*get_img_slot)(sg_shader_stage stage, const char* name);
	int (*get_smp_slot)(sg_shader_stage stage, const char* name);
	int (*get_uniformblock_slot)(sg_shader_stage stage, const char* ub_name);
	size_t (*get_uniformblock_size)(sg_shader_stage stage, const char* ub_name);
} pg_shader_internal_t;

pg_shader_t* pg_create_shader_internal(pg_ctx_t* ctx, pg_shader_internal_t internal);

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

#ifndef PICO_GFX_HASHTABLE_KEY_SIZE
#define PICO_GFX_HASHTABLE_KEY_SIZE 16
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

typedef enum
{
    PG_BUFFER_TYPE_VERTEX,
    PG_BUFFER_TYPE_INDEX,
} pg_buffer_type_t;

static sg_primitive_type pg_map_primitive(pg_primitive_t primitive);
static sg_blend_factor pg_map_blend_factor(pg_blend_factor_t factor);
static sg_blend_op pg_map_blend_eq(pg_blend_eq_t eq);
static sg_shader_stage pg_map_stage(pg_stage_t stage);
static sg_vertex_format pg_map_vertex_format(pg_vertex_format_t format);
static sg_usage pg_map_usage(pg_buffer_usage_t format);
static sg_buffer_type pg_map_buffer_type(pg_buffer_type_t type);

static void pg_log_sg(const char* tag,              // e.g. 'sg'
                      uint32_t log_level,           // 0=panic, 1=error, 2=warn, 3=info
                      uint32_t log_item_id,         // SG_LOGITEM_*
                      const char* message_or_null,  // a message string, may be nullptr in release mode
                      uint32_t line_nr,             // line number in sokol_gfx.h
                      const char* filename_or_null, // source filename, may be nullptr in release mode
                      void* user_data);

static void pg_log(const char* fmt, ...);

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
    pg_buffer_t*   index_buffer;
    pg_buffer_t*   buffers[PG_MAX_VERTEX_BUFFERS];
} pg_state_t;

struct pg_ctx_t
{
    void* mem_ctx;
    sg_swapchain swapchain;
    int window_width;
    int window_height;
    bool indexed;
    bool pass_active;
    pg_texture_t* target;
    sg_pass default_pass;
    pg_state_t state;
    pg_state_t state_stack[PICO_GFX_STACK_MAX_SIZE];
    int stack_size;
};

struct pg_pipeline_t
{
    pg_ctx_t* ctx;
    sg_pipeline handle;
    size_t element_size;
    bool indexed;
    pg_shader_t* shader;
};

struct pg_shader_t
{
    pg_ctx_t* ctx;
    const sg_shader_desc* desc;
    sg_shader handle;
    pg_shader_internal_t internal;
    pg_texture_t* textures[PG_MAX_TEXTURE_SLOTS];
    pg_sampler_t* samplers[PG_MAX_SAMPLER_SLOTS];
    pg_hashtable_t* uniform_blocks;
    pg_arena_t* arena;
};

typedef struct
{
    int        slot;
    pg_stage_t stage;
    void*      data;
    size_t     size;
} pg_uniform_block_t;

struct pg_texture_t
{
    pg_ctx_t* ctx;
    int width, height;
    bool target;
    sg_image handle;
    sg_image depth_handle;
    sg_attachments attachments;
};

struct pg_sampler_t
{
    pg_ctx_t* ctx;
    sg_sampler handle;
};

struct pg_buffer_t
{
    pg_ctx_t* ctx;
    sg_buffer handle;
    pg_buffer_type_t type;
    pg_buffer_usage_t usage;
    size_t count;
    size_t element_size;
    size_t size;
    size_t offset;
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
    pg_ctx_t* ctx = PICO_GFX_MALLOC(sizeof(pg_ctx_t), mem_ctx);

    if (!ctx) return NULL;

    memset(ctx, 0, sizeof(pg_ctx_t));

    ctx->mem_ctx = mem_ctx;
    ctx->window_width  = window_width;
    ctx->window_height = window_height;

    pg_reset_state(ctx);

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
    PICO_GFX_FREE(ctx, ctx->mem_ctx);
}

pg_backend_t pg_backend(void)
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
        //TODO: Do more research on swapchains
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
    (void)ctx;
    sg_commit();
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
    ctx->state.pipeline = pipeline;
}

void pg_reset_pipeline(pg_ctx_t* ctx)
{
    PICO_GFX_ASSERT(ctx);
    pg_set_pipeline(ctx, NULL);
}

void pg_bind_buffer(pg_ctx_t* ctx, int slot, pg_buffer_t* buffer)
{
    PICO_GFX_ASSERT(ctx);
    PICO_GFX_ASSERT(!buffer || buffer->type == PG_BUFFER_TYPE_VERTEX);

    PICO_GFX_ASSERT(slot >= 0);
    PICO_GFX_ASSERT(slot < PG_MAX_VERTEX_BUFFERS);

    ctx->state.buffers[slot] = buffer;
}

void pg_reset_buffers(pg_ctx_t* ctx)
{
    PICO_GFX_ASSERT(ctx);
    memset(&ctx->state.buffers, 0, sizeof(ctx->state.buffers));
}

void pg_set_index_buffer(pg_ctx_t* ctx, pg_buffer_t* buffer)
{
    PICO_GFX_ASSERT(ctx);
    PICO_GFX_ASSERT(!buffer || buffer->type == PG_BUFFER_TYPE_INDEX);
    ctx->state.index_buffer = buffer;
}

void pg_reset_index_buffer(pg_ctx_t* ctx)
{
    PICO_GFX_ASSERT(ctx);
    ctx->state.index_buffer = NULL;
}

void pg_bind_texture(pg_shader_t* shader, const char* name, pg_texture_t* texture)
{
    PICO_GFX_ASSERT(shader);
    PICO_GFX_ASSERT(name);

    int slot = shader->internal.get_img_slot(SG_SHADERSTAGE_FS, name);

    PICO_GFX_ASSERT(slot >= 0);
    PICO_GFX_ASSERT(slot < PG_MAX_TEXTURE_SLOTS);

    shader->textures[slot] = texture;
}

void pg_reset_textures(pg_shader_t* shader)
{
    PICO_GFX_ASSERT(shader);
    memset(shader->textures, 0, sizeof(shader->textures));
}

void pg_bind_sampler(pg_shader_t* shader, const char* name, pg_sampler_t* sampler)
{
    PICO_GFX_ASSERT(shader);
    PICO_GFX_ASSERT(name);

    int slot = shader->internal.get_smp_slot(SG_SHADERSTAGE_FS, name);

    PICO_GFX_ASSERT(slot >= 0);
    PICO_GFX_ASSERT(slot < PG_MAX_TEXTURE_SLOTS);

    shader->samplers[slot] = sampler;
}

void pg_reset_samplers(pg_shader_t* shader)
{
    PICO_GFX_ASSERT(shader);
    memset(shader->samplers, 0, sizeof(shader->samplers));
}

void pg_reset_state(pg_ctx_t* ctx)
{
    PICO_GFX_ASSERT(ctx);

    memset(&ctx->state, 0, sizeof(pg_state_t));

    pg_reset_clear_color(ctx);
    pg_reset_pipeline(ctx);
    pg_reset_viewport(ctx);
    pg_reset_scissor(ctx);
    pg_reset_buffers(ctx);
    pg_reset_index_buffer(ctx);
}

static void pg_set_attributes(const pg_pipeline_layout_t* layout, sg_pipeline_desc* desc)
{
    for (int slot = 0; slot < PG_MAX_VERTEX_ATTRIBUTES; slot++)
    {
        if (layout->attrs[slot].format != PG_VERTEX_FORMAT_INVALID)
        {
            desc->layout.attrs[slot] = (sg_vertex_attr_state)
            {
                .format = pg_map_vertex_format(layout->attrs[slot].format),
                .offset = layout->attrs[slot].offset,
                .buffer_index = layout->attrs[slot].buffer_index,
            };
        }
    }
}

static void pg_set_buffers(const pg_pipeline_layout_t* layout, sg_pipeline_desc* desc)
{
    for (int slot = 0; slot < PG_MAX_VERTEX_BUFFERS; slot++)
    {
        int step = layout->bufs[slot].step;

        if (layout->bufs[slot].instanced)
        {
            desc->layout.buffers[slot] = (sg_vertex_buffer_layout_state)
            {
                .step_func = SG_VERTEXSTEP_PER_INSTANCE,
            };
        }

        desc->layout.buffers[slot].step_rate = (step >= 1) ? step : 1;
    }
}

pg_pipeline_t* pg_create_pipeline(pg_ctx_t* ctx,
                                  pg_shader_t* shader,
                                  const pg_pipeline_opts_t* opts)
{
    PICO_GFX_ASSERT(shader);
    PICO_GFX_ASSERT(opts);

    pg_pipeline_t* pipeline = PICO_GFX_MALLOC(sizeof(pg_pipeline_t), ctx->mem_ctx);

    sg_pipeline_desc desc = { 0 };

    pg_set_attributes(&opts->layout, &desc);
    pg_set_buffers(&opts->layout, &desc);

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

    //TODO: Culling

    desc.face_winding = SG_FACEWINDING_CCW;
    desc.shader = shader->handle;

    pipeline->ctx = ctx;
    pipeline->handle = sg_make_pipeline(&desc);
    pipeline->indexed = opts->indexed;
    pipeline->shader = shader;

    PICO_GFX_ASSERT(sg_query_pipeline_state(pipeline->handle) == SG_RESOURCESTATE_VALID);

    return pipeline;
}

void pg_destroy_pipeline(pg_pipeline_t* pipeline)
{
    PICO_GFX_ASSERT(pipeline);
    sg_destroy_pipeline(pipeline->handle);
    PICO_GFX_FREE(pipeline, pipeline->ctx->mem_ctx);
}

pg_shader_t* pg_get_pipeline_shader(const pg_pipeline_t* pipeline)
{
    PICO_GFX_ASSERT(pipeline);
    return pipeline->shader;
}

pg_shader_t* pg_create_shader_internal(pg_ctx_t* ctx, pg_shader_internal_t internal)
{
    pg_shader_t* shader = PICO_GFX_MALLOC(sizeof(pg_shader_t), ctx->mem_ctx);

    pg_reset_textures(shader);
    pg_reset_samplers(shader);

    shader->ctx = ctx;
    shader->internal = internal;
    shader->desc = internal.get_shader_desc(sg_query_backend());

    PICO_GFX_ASSERT(shader->desc);

    shader->handle = sg_make_shader(shader->desc);

    shader->uniform_blocks = pg_hashtable_new(16, PICO_GFX_HASHTABLE_KEY_SIZE,
                                              sizeof(pg_uniform_block_t),
                                              ctx->mem_ctx);

    shader->arena = pg_arena_new(512, ctx->mem_ctx);

    return shader;
}

void pg_destroy_shader(pg_shader_t* shader)
{
    PICO_GFX_ASSERT(shader);
    sg_destroy_shader(shader->handle);
    pg_hashtable_free(shader->uniform_blocks);
    pg_arena_free(shader->arena);
    PICO_GFX_FREE(shader, shader->ctx->mem_ctx);
}

uint32_t pg_get_shader_id(const pg_shader_t* shader)
{
    PICO_GFX_ASSERT(shader);
    return shader->handle.id;
}

void pg_alloc_uniform_block(pg_shader_t* shader, pg_stage_t stage, const char* name)
{
    PICO_GFX_ASSERT(shader);
    PICO_GFX_ASSERT(name);

    size_t size = shader->internal.get_uniformblock_size(pg_map_stage(stage), name);

    pg_uniform_block_t block =
    {
        .slot  = shader->internal.get_uniformblock_slot(pg_map_stage(stage), name),
        .stage = stage,
        .data  = pg_arena_alloc(shader->arena, size),
        .size  = size,
    };

    pg_hashtable_put(shader->uniform_blocks, name, &block);
}

void pg_set_uniform_block(pg_shader_t* shader,
                          const char* name,
                          const void* data)
{
    PICO_GFX_ASSERT(shader);
    PICO_GFX_ASSERT(name);
    PICO_GFX_ASSERT(data);

    pg_uniform_block_t* block = pg_hashtable_get(shader->uniform_blocks, name);

    PICO_GFX_ASSERT(block);

    memcpy(block->data, data, block->size);
}

pg_texture_t* pg_create_texture(pg_ctx_t* ctx,
                                int width, int height,
                                const uint8_t* data, size_t size,
                                const pg_texture_opts_t* opts)
{
    PICO_GFX_ASSERT(width > 0);
    PICO_GFX_ASSERT(height > 0);
    PICO_GFX_ASSERT(data);
    PICO_GFX_ASSERT(size > 0);

    if (opts == NULL)
        opts = &(pg_texture_opts_t){ 0 };

    PICO_GFX_ASSERT(opts->mipmaps >= 0);

    pg_texture_t* texture = PICO_GFX_MALLOC(sizeof(pg_texture_t), ctx->mem_ctx);

    sg_image_desc desc = { 0 };

    desc.pixel_format = SG_PIXELFORMAT_RGBA8;

    desc.width  = texture->width  = width;
    desc.height = texture->height = height;

    desc.num_mipmaps = opts->mipmaps;
    desc.data.subimage[0][0] = (sg_range){ .ptr = data, .size = size };

    texture->ctx = ctx;
    texture->target = false;
    texture->handle = sg_make_image(&desc);

    PICO_GFX_ASSERT(sg_query_image_state(texture->handle) == SG_RESOURCESTATE_VALID);

    return texture;
}

pg_texture_t* pg_create_render_texture(pg_ctx_t* ctx,
                                       int width, int height,
                                       const pg_texture_opts_t* opts)
{
    PICO_GFX_ASSERT(width > 0);
    PICO_GFX_ASSERT(height > 0);

    if (opts == NULL)
        opts = &(pg_texture_opts_t){ 0 };

    PICO_GFX_ASSERT(opts->mipmaps >= 0);

    pg_texture_t* texture = PICO_GFX_MALLOC(sizeof(pg_texture_t), ctx->mem_ctx);

    sg_image_desc desc = { 0 };

    desc.render_target = true;
    desc.pixel_format = SG_PIXELFORMAT_RGBA8;

    desc.width  = texture->width  = width;
    desc.height = texture->height = height;

    desc.num_mipmaps = opts->mipmaps;

    texture->ctx = ctx;
    texture->handle = sg_make_image(&desc);
    texture->target = true;

    PICO_GFX_ASSERT(sg_query_image_state(texture->handle) == SG_RESOURCESTATE_VALID);

    desc.pixel_format = SG_PIXELFORMAT_DEPTH;
    texture->depth_handle = sg_make_image(&desc);

    PICO_GFX_ASSERT(sg_query_image_state(texture->depth_handle) == SG_RESOURCESTATE_VALID);

    //TODO: Research multiple color attachments
    texture->attachments = sg_make_attachments(&(sg_attachments_desc)
    {
        .colors[0].image = texture->handle,
        .depth_stencil.image = texture->depth_handle,
    });

    return texture;
}

void pg_destroy_texture(pg_texture_t* texture)
{
    if (texture->target)
    {
        sg_destroy_image(texture->depth_handle);
    }

    sg_destroy_image(texture->handle);
    PICO_GFX_FREE(texture, texture->ctx->mem_ctx);
}

void pg_update_texture(pg_texture_t* texture, char* data, int width, int height)
{
    // NOTE: Replaces all existing data
    sg_image_data img_data = { 0 };
    img_data.subimage[0][0].ptr = data;
    img_data.subimage[0][0].size = (size_t)(width * height);
    sg_update_image(texture->handle, &img_data);
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

pg_sampler_t* pg_create_sampler(pg_ctx_t* ctx, const pg_sampler_opts_t* opts)
{
    if (opts == NULL)
        opts = &(pg_sampler_opts_t){ 0 };

    pg_sampler_t* sampler = PICO_GFX_MALLOC(sizeof(pg_sampler_t), ctx->mem_ctx);

    sg_sampler_desc desc = { 0 };

    desc.min_filter = (opts->smooth) ? SG_FILTER_LINEAR : SG_FILTER_NEAREST;
    desc.mag_filter = (opts->smooth) ? SG_FILTER_LINEAR : SG_FILTER_NEAREST;

    desc.wrap_u = (opts->repeat_u) ? SG_WRAP_REPEAT : SG_WRAP_CLAMP_TO_EDGE;
    desc.wrap_v = (opts->repeat_v) ? SG_WRAP_REPEAT : SG_WRAP_CLAMP_TO_EDGE;

    sampler->ctx = ctx;
    sampler->handle = sg_make_sampler(&desc);

    return sampler;
}

void pg_destroy_sampler(pg_sampler_t* sampler)
{
    sg_destroy_sampler(sampler->handle);
    PICO_GFX_FREE(sampler, sampler->ctx->mem_ctx);
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

        sg_range range = { .ptr = block->data, .size = block->size };

        sg_shader_stage stage = pg_map_stage(block->stage);

        sg_apply_uniforms(stage, block->slot, &range);
    }
}

pg_buffer_t* pg_create_vertex_buffer(pg_ctx_t* ctx,
                                     pg_buffer_usage_t usage,
                                     const void* data,
                                     size_t count,
                                     size_t max_elements,
                                     size_t element_size)
{
    PICO_GFX_ASSERT(ctx);

    pg_buffer_t* buffer = PICO_GFX_MALLOC(sizeof(pg_buffer_t), ctx->mem_ctx);

    buffer->ctx = ctx;
    buffer->type = PG_BUFFER_TYPE_VERTEX;
    buffer->usage = usage;
    buffer->count = count;
    buffer->element_size = element_size;
    buffer->size = max_elements * element_size;
    buffer->offset = 0;

    buffer->handle = sg_make_buffer(&(sg_buffer_desc)
    {
        .type  = SG_BUFFERTYPE_VERTEXBUFFER,
        .usage = pg_map_usage(usage),
        .data  = { .ptr = data, .size = count * element_size },
        .size  = buffer->size
    });

    PICO_GFX_ASSERT(sg_query_buffer_state(buffer->handle) == SG_RESOURCESTATE_VALID);

    return buffer;
}

pg_buffer_t* pg_create_index_buffer(pg_ctx_t* ctx,
                                    pg_buffer_usage_t usage,
                                    const void* data,
                                    size_t count,
                                    size_t max_elements)
{
    PICO_GFX_ASSERT(ctx);

    pg_buffer_t* buffer = PICO_GFX_MALLOC(sizeof(pg_buffer_t), ctx->mem_ctx);

    buffer->ctx = ctx;
    buffer->type = PG_BUFFER_TYPE_INDEX;
    buffer->usage = usage;
    buffer->count = count;
    buffer->size = max_elements * sizeof(uint32_t);
    buffer->offset = 0;

    buffer->handle = sg_make_buffer(&(sg_buffer_desc)
    {
        .type  = SG_BUFFERTYPE_INDEXBUFFER,
        .usage = pg_map_usage(usage),
        .data  = { .ptr = data, .size = count * sizeof(uint32_t) },
        .size  = buffer->size
    });

    PICO_GFX_ASSERT(sg_query_buffer_state(buffer->handle) == SG_RESOURCESTATE_VALID);

    return buffer;
}

void pg_update_buffer(pg_buffer_t* buffer, void* data, size_t count)
{
    PICO_GFX_ASSERT(buffer);
    PICO_GFX_ASSERT(data);
    PICO_GFX_ASSERT(count > 0);

    sg_update_buffer(buffer->handle, &(sg_range)
    {
        .ptr = data,
        .size = count * buffer->element_size
    });

    buffer->count = count;
    buffer->offset = 0;
}

int pg_append_buffer(pg_buffer_t* buffer, void* data, size_t count)
{
    PICO_GFX_ASSERT(buffer);
    PICO_GFX_ASSERT(data);
    PICO_GFX_ASSERT(count > 0);

    int offset = sg_append_buffer(buffer->handle, &(sg_range)
    {
        .ptr = data,
        .size = count * buffer->element_size
    });

    buffer->count = count;
    buffer->offset = offset;

    return offset;
}

int pg_get_buffer_offset(pg_buffer_t* buffer)
{
    PICO_GFX_ASSERT(buffer);
    return buffer->offset;
}

void pg_set_buffer_offset(pg_buffer_t* buffer, int offset)
{
    PICO_GFX_ASSERT(buffer);
    buffer->offset = offset;
}

void pg_reset_buffer(pg_buffer_t* buffer)
{
    PICO_GFX_ASSERT(buffer);

    sg_destroy_buffer(buffer->handle);

    buffer->handle = sg_make_buffer(&(sg_buffer_desc)
    {
        .type  = pg_map_buffer_type(buffer->type),
        .usage = pg_map_usage(buffer->usage),
        .data  = { .ptr = NULL, .size = 0 },
        .size  = buffer->size
    });

    PICO_GFX_ASSERT(sg_query_buffer_state(buffer->handle) == SG_RESOURCESTATE_VALID);
}

void pg_destroy_buffer(pg_buffer_t* buffer)
{
    PICO_GFX_ASSERT(buffer);
    sg_destroy_buffer(buffer->handle);
    PICO_GFX_FREE(buffer, buffer->ctx->mem_ctx);
}

static void pg_apply_view_state(const pg_ctx_t* ctx)
{
    const pg_rect_t* vp_rect = &ctx->state.viewport;
    sg_apply_viewport(vp_rect->x, vp_rect->y, vp_rect->width, vp_rect->height, true);

    const pg_rect_t* s_rect = &ctx->state.scissor;
    sg_apply_scissor_rect(s_rect->x, s_rect->y, s_rect->width, s_rect->height, true);
}

static void pg_apply_textures(const pg_shader_t* shader, sg_bindings* bindings)
{
    for (int i = 0; i < PG_MAX_TEXTURE_SLOTS; i++)
    {
        if (!shader->textures[i])
            continue;

        bindings->fs.images[i] = shader->textures[i]->handle;
    }
}

static void pg_apply_samplers(const pg_shader_t* shader, sg_bindings* bindings)
{
    for (int i = 0; i < PG_MAX_SAMPLER_SLOTS; i++)
    {
        if (!shader->samplers[i])
            continue;

        bindings->fs.samplers[i] = shader->samplers[i]->handle;
    }
}

static void pg_apply_buffers(const pg_ctx_t* ctx, sg_bindings* bindings)
{
    pg_buffer_t* const* buffers = ctx->state.buffers;

    for (int slot = 0; buffers[slot] != NULL; slot++)
    {
        bindings->vertex_buffer_offsets[slot] = buffers[slot]->offset;
        bindings->vertex_buffers[slot] = buffers[slot]->handle;
    }
}

void pg_draw(const pg_ctx_t* ctx, size_t start, size_t count, size_t instances)
{
    PICO_GFX_ASSERT(ctx);
    PICO_GFX_ASSERT(ctx->pass_active);

    sg_bindings bindings = { 0 };

    pg_pipeline_t* pipeline = ctx->state.pipeline;

    pg_apply_textures(pipeline->shader, &bindings);
    pg_apply_samplers(pipeline->shader, &bindings);

    pg_apply_buffers(ctx, &bindings);
    pg_apply_view_state(ctx);

    if (ctx->state.index_buffer)
    {
        bindings.index_buffer_offset = ctx->state.index_buffer->offset;
        bindings.index_buffer = ctx->state.index_buffer->handle;
    }

    sg_apply_pipeline(pipeline->handle);
    sg_apply_bindings(&bindings);
    pg_apply_uniforms(pipeline->shader);

    sg_draw(start, count, instances);
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
        default: PICO_GFX_ASSERT(false); return SG_PRIMITIVETYPE_TRIANGLES;
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
        default: PICO_GFX_ASSERT(false); return SG_BLENDFACTOR_ONE;
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
        default: PICO_GFX_ASSERT(false); return SG_BLENDOP_ADD;
    }
}

static sg_shader_stage pg_map_stage(pg_stage_t stage)
{
    switch (stage)
    {
        case PG_STAGE_VS: return SG_SHADERSTAGE_VS;
        case PG_STAGE_FS: return SG_SHADERSTAGE_FS;
        default: PICO_GFX_ASSERT(false); return 0;
    }
}

static sg_vertex_format pg_map_vertex_format(pg_vertex_format_t format)
{
    switch (format)
    {
        case PG_VERTEX_FORMAT_INVALID:   return SG_VERTEXFORMAT_INVALID;
        case PG_VERTEX_FORMAT_FLOAT:     return SG_VERTEXFORMAT_FLOAT;
        case PG_VERTEX_FORMAT_FLOAT2:    return SG_VERTEXFORMAT_FLOAT2;
        case PG_VERTEX_FORMAT_FLOAT3:    return SG_VERTEXFORMAT_FLOAT3;
        case PG_VERTEX_FORMAT_FLOAT4:    return SG_VERTEXFORMAT_FLOAT4;
        case PG_VERTEX_FORMAT_BYTE4:     return SG_VERTEXFORMAT_BYTE4;
        case PG_VERTEX_FORMAT_BYTE4N:    return SG_VERTEXFORMAT_BYTE4N;
        case PG_VERTEX_FORMAT_UBYTE4:    return SG_VERTEXFORMAT_UBYTE4;
        case PG_VERTEX_FORMAT_UBYTE4N:   return SG_VERTEXFORMAT_UBYTE4N;
        case PG_VERTEX_FORMAT_SHORT2:    return SG_VERTEXFORMAT_SHORT2;
        case PG_VERTEX_FORMAT_SHORT2N:   return SG_VERTEXFORMAT_SHORT2N;
        case PG_VERTEX_FORMAT_USHORT2N:  return SG_VERTEXFORMAT_USHORT2N;
        case PG_VERTEX_FORMAT_SHORT4:    return SG_VERTEXFORMAT_USHORT2N;
        case PG_VERTEX_FORMAT_SHORT4N:   return SG_VERTEXFORMAT_SHORT4;
        case PG_VERTEX_FORMAT_USHORT4N:  return SG_VERTEXFORMAT_SHORT4N;
        case PG_VERTEX_FORMAT_UINT10_N2: return SG_VERTEXFORMAT_USHORT4N;
        case PG_VERTEX_FORMAT_HALF2:     return SG_VERTEXFORMAT_HALF2;
        case PG_VERTEX_FORMAT_HALF4:     return SG_VERTEXFORMAT_HALF4;
        default: PICO_GFX_ASSERT(false); return SG_VERTEXFORMAT_INVALID;
    }
}

static sg_usage pg_map_usage(pg_buffer_usage_t format)
{
    switch (format)
    {
        case PG_USAGE_STATIC:  return SG_USAGE_IMMUTABLE;
        case PG_USAGE_DYNAMIC: return SG_USAGE_DYNAMIC;
        case PG_USAGE_STREAM:  return SG_USAGE_STREAM;
        default: PICO_GFX_ASSERT(false); return SG_USAGE_IMMUTABLE;
    }
}

static sg_buffer_type pg_map_buffer_type(pg_buffer_type_t type)
{
    switch (type)
    {
        case PG_BUFFER_TYPE_VERTEX: return SG_BUFFERTYPE_VERTEXBUFFER;
        case PG_BUFFER_TYPE_INDEX:  return SG_BUFFERTYPE_INDEXBUFFER;
        default: PICO_GFX_ASSERT(false); return SG_BUFFERTYPE_VERTEXBUFFER;
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

            strncpy(entry->key, key, ht->key_size);
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
