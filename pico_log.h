/** @file picolog.h
 * picolog is a minimal, yet flexible logging framework written in C99.
 */

/*=============================================================================
 * MIT License
 *
 * Copyright (c) 2020 James McLean
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
=============================================================================*/

#ifndef PICO_LOG_H
#define PICO_LOG_H

#ifndef PL_ASSERT
#include <assert.h>  // assert
#endif

#include <stdarg.h>  // ...
#include <stdbool.h> // bool, true, false
#include <stddef.h>  // NULL, size_t
#include <stdio.h>   // FILE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * These codes allow different layers of granularity when logging. See the
 * documentation of the `pl_set_level` function for more information.
 */
typedef enum
{
    PL_LEVEL_TRACE = 0,
    PL_LEVEL_DEBUG,
    PL_LEVEL_INFO,
    PL_LEVEL_WARN,
    PL_LEVEL_ERROR,
    PL_LEVEL_FATAL,
    PL_LEVEL_COUNT
} pl_level_t;

/**
 * Appender function definition. An appender writes a log entry to an output
 * stream. This could be the console, a file, a network connection, etc...
 */
typedef void (*pl_appender_fn)(const char* p_entry, void* p_udata);

/**
 *  Lock function definition. This is called during pl_write. Adapted
    from https://github.com/rxi/log.c/blob/master/src/log.h
 */
typedef void (*pl_lock_fn)(bool lock, void *p_udata);

/**
 * Identifies a registered appender.
 */
typedef size_t pl_id_t;

/**
  * Converts a string to the corresponding log level
  */
bool pl_str_level(const char* str, pl_level_t* level);

/**
 * Enables logging. NOTE: Logging is enabled by default.
 */
void pl_enable(void);

/**
 * Disables logging.
 */
void pl_disable(void);

/**
 * Registers (adds appender to logger) and enables the specified appender. An
 * appender writes a log entry to an output stream. This could be a console,
 * a file, a network connection, etc...
 *
 * @param p_appender  Pointer to the appender function to register. An appender
 *                    function has the signature,
 *                    `void appender_func(const char* p_entry, void* p_udata)`
 *
 * @param level       The appender's log level
 *
 * @param p_udata A pointer supplied to the appender function when writing
 *                    a log entry. This pointer is not modified by the logger.
 *                    If not required, pass in NULL for this parameter.
 *
 * @return            An identifier for the appender. This ID is valid until the
 *                    appender is unregistered.
 */
pl_id_t pl_add_appender(pl_appender_fn p_appender,
                        pl_level_t level, void* p_udata);

/**
 * Registers an output stream appender.
 *
 * @param p_stream The output stream to write to
 * @param level  The appender's log level
 *
 * @return       An identifier for the appender. This ID is valid until the
 *               appender is unregistered.
 */
pl_id_t pl_add_stream(FILE* p_stream, pl_level_t level);

/**
 * Unregisters appender (removes the appender from the logger).
 *
 * @param id The appender to unregister
 */
void pl_remove_appender(pl_id_t id);

/**
 * Enables the specified appender. NOTE: Appenders are enabled by default after
 * registration.
 *
 * @param id The appender to enable.
 */
void pl_enable_appender(pl_id_t id);

/**
 * Disables the specified appender.
 *
 * @param id The appender to disable
 */
void pl_disable_appender(pl_id_t id);

/**
 * Sets the locking function.
 */
void pl_set_lock(pl_id_t id, pl_lock_fn p_lock, void* p_udata);

/**
 * Sets the logging level. Only those messages of equal or higher priority
 * (severity) than this value will be logged.
 *
 * @param level The new appender logging threshold.
 */
void pl_set_level(pl_id_t id, pl_level_t level);

/**
 * Sets the appender timestamp format according to:
 * https://man7.org/linux/man-pages/man3/strftime.3.html
 *
 * @param id The appender id
 * @param fmt The time format
 */
void pl_set_time_fmt(pl_id_t id, const char* fmt);

/**
 * Turns colors ouput on or off for the specified appender.
 * NOTE: Off by default.
 *
 * @param id The appender id
 * @param b_enabled On if true
 */
void pl_colors_enabled(pl_id_t id, bool b_enabled);

/**
 * Turns timestamp reporting on/off for the specified appender.
 * NOTE: Off by default
 *
 * @param id The appender id
 * @param b_enabled On if true
 */
void pl_timestamp_enabled(pl_id_t id, bool b_enabled);

/**
 * Turns log level reporting on/off for the specified appender.
 * NOTE: On by default.
 *
 * @param id The appender id
 * @param b_enabled On if true
 */
void pl_level_enabled(pl_id_t id, bool b_enabled);

/**
 * Turns filename and line number reporting on/off for the specified appender.
 * NOTE: Off by default.
 *
 * @param id The appender id
 * @param b_enabled On if true
 */
void pl_file_enabled(pl_id_t id, bool b_enabled);

/**
 * Turns function reporting on/off for the specified appender.
 * NOTE: Off by default.
 *
 * @param id The appender id
 * @param b_enabled On if true
 */
void pl_function_enabled(pl_id_t id, bool b_enabled);

/**
 * Writes a TRACE level message to the log. Usage is similar to printf (i.e.
 * PL_TRACE(format, args...))
 */
#define pl_trace(...) \
        pl_write(PL_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * Writes a DEBUG level message to the log. Usage is similar to printf (i.e.
 * PL_DEBUG(format, args...))
 */
#define pl_debug(...) \
        pl_write(PL_LEVEL_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * Writes an INFO level message to the log. Usage is similar to printf (i.e.
 * PL_INFO(format, args...))
 */
#define pl_info(...) \
        pl_write(PL_LEVEL_INFO,  __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * Writes a WARN level message to the log. Usage is similar to printf (i.e.
 * PL_WARN(format, args...))
 */
#define pl_warn(...) \
        pl_write(PL_LEVEL_WARN,  __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * Writes a ERROR level message to the log. Usage is similar to printf (i.e.
 * PL_ERROR(format, args...))
 */
#define pl_error(...) \
        pl_write(PL_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * Writes a FATAL level message to the log.. Usage is similar to printf (i.e.
 * PL_FATAL(format, args...))
 */
#define pl_fatal(...) \
        pl_write(PL_LEVEL_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)


/**
 * WARNING: It is inadvisable to call this function directly. Use the macros
 * instead.
 */
void pl_write(pl_level_t level,
                const char* file,
                unsigned line,
                const char* func,
                const char* p_fmt, ...);


#ifdef __cplusplus
}
#endif

#endif // PICO_LOG_H

#ifdef PL_IMPLEMENTATION

#include <time.h>
#include <string.h>

/*
 * Configuration constants/macros.
 */
#ifndef PL_MAX_APPENDERS
#define PL_MAX_APPENDERS 16
#endif

#ifndef PL_MAX_MSG_LENGTH
#define PL_MAX_MSG_LENGTH 1024
#endif

#ifndef PL_ASSERT
#define PL_ASSERT(expr) assert(expr)
#endif

/*
 * Log entry component maximum sizes. These have been chosen to be overly
 * generous powers of 2 for the sake of safety and simplicity.
 */

#define PL_TIMESTAMP_LEN 64
#define PL_LEVEL_LEN     32
#define PL_FILE_LEN      512
#define PL_FUNC_LEN      32
#define PL_MSG_LEN       PL_MAX_MSG_LENGTH
#define PL_BREAK_LEN     1

#define PL_ENTRY_LEN (PL_TIMESTAMP_LEN  + \
                        PL_LEVEL_LEN      + \
                        PL_FILE_LEN       + \
                        PL_FUNC_LEN       + \
                        PL_MSG_LEN        + \
                        PL_BREAK_LEN)

#define PL_TIME_FMT_LEN 32
#define PL_TIME_FMT     "%d/%m/%g %H:%M:%S"

#define PL_TERM_CODE  0x1B
#define PL_TERM_RESET "[0m"
#define PL_TERM_GRAY  "[90m"

static bool   gb_initialized   = false; // True if logger is initialized
static bool   gb_enabled       = true;  // True if logger is enabled
static size_t g_appender_count = 0;         // Number of appenders

/*
 * Logger level strings indexed by level ID (pl_level_t).
 */
static const char* const level_str[] =
{
    "TRACE",
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL",
    0
};

/*
 * Logger level strings indexed by level ID (pl_level_t).
 */
static const char* const level_str_formatted[] =
{
    "TRACE",
    "DEBUG",
    "INFO ",
    "WARN ",
    "ERROR",
    "FATAL",
    0
};

// Appropriated from https://github.com/rxi/log.c (MIT licensed)
static const char* level_color[] =
{
    "[94m", "[36m", "[32m", "[33m", "[31m", "[35m", NULL
};

/*
 * Appender pointer and metadata.
 */
typedef struct
{
    pl_appender_fn p_appender;
    void*          p_udata;
    bool           b_enabled;
    pl_level_t     level;
    char           p_time_fmt[PL_TIME_FMT_LEN];
    bool           b_colors;
    bool           b_timestamp;
    bool           b_level;
    bool           b_file;
    bool           b_func;
    pl_lock_fn     p_lock;
    void*          p_lock_udata;
} appender_info_t;

/*
 * Array of appenders.
 */
static appender_info_t gp_appenders[PL_MAX_APPENDERS];

/*
 * Initializes the logger provided it has not been initialized.
 */
static void
try_init ()
{
    if (gb_initialized)
    {
        return;
    }

    for (int i = 0; i < PL_MAX_APPENDERS; i++)
    {
        gp_appenders[i].p_appender = NULL;
    }

    gb_initialized = true;
}

static bool appender_exists(pl_id_t id)
{
    return (id < PL_MAX_APPENDERS && NULL != gp_appenders[id].p_appender);
}

static bool appender_enabled(pl_id_t id)
{
    return appender_exists(id) && gp_appenders[id].b_enabled;
}

bool pl_str_level(const char* str, pl_level_t* level)
{
    if (!level)
        return false;

    for (size_t i = 0; level_str[i]; i++)
    {
        if (0 == strcmp(str, level_str[i]))
        {
            *level = (pl_level_t)i;
            return true;
        }
    }

    return false;
}

void
pl_enable (void)
{
    gb_enabled = true;
}

void
pl_disable (void)
{
    gb_enabled = false;
}

pl_id_t
pl_add_appender (pl_appender_fn p_appender,
                   pl_level_t level,
                   void* p_udata)
{
    // Initialize logger if neccesary
    try_init();

    // Check if there is space for a new appender.
    PL_ASSERT(g_appender_count < PL_MAX_APPENDERS);

    // Ensure level is valid
    PL_ASSERT(level >= 0 && level < PL_LEVEL_COUNT);

    // Iterate through appender array and find an empty slot.
    for (int i = 0; i < PL_MAX_APPENDERS; i++)
    {
        if (NULL == gp_appenders[i].p_appender)
        {
            // Store and enable appender
            gp_appenders[i].p_appender   = p_appender;
            gp_appenders[i].level        = level;
            gp_appenders[i].p_udata      = p_udata;
            gp_appenders[i].level        = PL_LEVEL_INFO;
            gp_appenders[i].b_enabled    = true;
            gp_appenders[i].b_colors     = false;
            gp_appenders[i].b_level      = true;
            gp_appenders[i].b_timestamp  = false;
            gp_appenders[i].b_file       = false;
            gp_appenders[i].b_func       = false;
            gp_appenders[i].p_lock       = NULL;
            gp_appenders[i].p_lock_udata = NULL;

            strncpy(gp_appenders[i].p_time_fmt, PL_TIME_FMT, PL_TIME_FMT_LEN);

            g_appender_count++;

            return (pl_id_t)i;
        }
    }

    // This should never happen
    PL_ASSERT(false);
    return 0;
}

static void
stream_appender (const char* p_entry, void* p_udata)
{
    FILE* stream = (FILE*)p_udata;
    fprintf(stream, "%s", p_entry);
    fflush(stream);
}

pl_id_t
pl_add_stream (FILE* stream, pl_level_t level)
{
    // Stream must not be NULL
    PL_ASSERT(NULL != stream);

    return pl_add_appender(stream_appender, level, stream);
}

void
pl_remove_appender (pl_id_t id)
{
    // Initialize logger if neccesary
    try_init();

    // Ensure appender is registered
    PL_ASSERT(appender_exists(id));

    // Reset appender with given ID
    gp_appenders[id].p_appender = NULL;

    g_appender_count--;
}

void
pl_enable_appender (pl_id_t id)
{
    // Initialize logger if neccesary
    try_init();

    // Ensure appender is registered
    PL_ASSERT(appender_exists(id));

    // Enable appender
    gp_appenders[id].b_enabled = true;
}

void
pl_disable_appender (pl_id_t id)
{
    // Initialize logger if neccesary
    try_init();

    // Ensure appender is registered
    PL_ASSERT(appender_exists(id));

    // Disable appender
    gp_appenders[id].b_enabled = false;
}

void pl_set_lock(pl_id_t id, pl_lock_fn p_lock, void* p_udata)
{
    // Ensure lock function is initialized
    PL_ASSERT(NULL != p_lock);

    // Ensure appender is registered
    try_init();

    // Ensure appender is registered
    PL_ASSERT(appender_exists(id));

    gp_appenders[id].p_lock = p_lock;
    gp_appenders[id].p_lock_udata = p_udata;
}


void
pl_set_level (pl_id_t id, pl_level_t level)
{
    // Initialize logger if neccesary
    try_init();

    // Ensure appender is registered
    PL_ASSERT(appender_exists(id));

    // Ensure level is valid
    PL_ASSERT(level >= 0 && level < PL_LEVEL_COUNT);

    // Set the level
    gp_appenders[id].level = level;
}

void
pl_set_time_fmt (pl_id_t id, const char* fmt)
{
    // Initialize logger if neccesary
    try_init();

    // Ensure appender is registered
    PL_ASSERT(appender_exists(id));

    // Copy the time string
    strncpy(gp_appenders[id].p_time_fmt, fmt, PL_TIME_FMT_LEN);
}

void
pl_colors_enabled (pl_id_t id, bool b_enabled)
{
    // Initialize logger if neccesary
    try_init();

    // Ensure appender is registered
    PL_ASSERT(appender_exists(id));

    // Disable appender
    gp_appenders[id].b_colors = b_enabled;
}

void
pl_timestamp_enabled (pl_id_t id, bool b_enabled)
{
    // Initialize logger if neccesary
    try_init();

    // Ensure appender is registered
    PL_ASSERT(appender_exists(id));

    // Turn timestamp on
    gp_appenders[id].b_timestamp = b_enabled;
}

void
pl_level_enabled (pl_id_t id, bool b_enabled)
{
    // Initialize logger if neccesary
    try_init();

    // Ensure appender is registered
    PL_ASSERT(appender_exists(id));

    // Turn level reporting on
    gp_appenders[id].b_level = b_enabled;
}

void
pl_file_enabled (pl_id_t id, bool b_enabled)
{
    // Initialize logger if neccesary
    try_init();

    // Ensure appender is registered
    PL_ASSERT(appender_exists(id));

    // Turn file reporting on
    gp_appenders[id].b_file = b_enabled;
}

void
pl_function_enabled (pl_id_t id, bool b_enabled)
{
    // Initialize logger if neccesary
    try_init();

    // Ensure appender is registered
    PL_ASSERT(appender_exists(id));

    // Turn file reporting on
    gp_appenders[id].b_func = b_enabled;
}

/*
 * Formats the current time as as string.
 */
static char*
time_str (const char* p_time_fmt, char* p_str, size_t len)
{
    time_t now = time(0);
    size_t ret = strftime(p_str, len, p_time_fmt, localtime(&now));

    PL_ASSERT(ret > 0);

    return p_str;
}

static void
append_timestamp (char* p_entry_str, const char* p_time_fmt)
{
    char p_time_str[PL_TIMESTAMP_LEN + 1];
    char p_tmp_str[PL_TIMESTAMP_LEN + 1];

    snprintf(p_time_str, PL_TIMESTAMP_LEN, "%s ",
             time_str(p_time_fmt, p_tmp_str, PL_TIMESTAMP_LEN));

    strncat(p_entry_str, p_time_str, PL_TIMESTAMP_LEN);
}

static void
append_level (char* p_entry_str, pl_level_t level, bool b_colors)
{
    char p_level_str[PL_LEVEL_LEN];

    if (b_colors)
    {
        snprintf(p_level_str, sizeof(p_level_str), "%c%s%s %c%s",
        PL_TERM_CODE, level_color[level],
        level_str_formatted[level],
        PL_TERM_CODE, PL_TERM_RESET);
    }
    else
    {
        snprintf(p_level_str, sizeof(p_level_str), "%s ", level_str[level]);
    }

    strncat(p_entry_str, p_level_str, PL_LEVEL_LEN);
}

static void
append_file(char* p_entry_str, const char* file, unsigned line, bool b_colors)
{
    char p_file_str[PL_FILE_LEN];

    if (b_colors)
    {
        snprintf(p_file_str, sizeof(p_file_str), "%c%s%s:%u%c%s ",
                 PL_TERM_CODE, PL_TERM_GRAY,
                 file, line,
                 PL_TERM_CODE, PL_TERM_RESET);

    }
    else
    {
        snprintf(p_file_str, sizeof(p_file_str), "%s:%u ", file, line);
    }

    strncat(p_entry_str, p_file_str, PL_FILE_LEN);
}

static void
append_func(char* p_entry_str, const char* func, bool b_colors)
{
   char p_func_str[PL_FUNC_LEN];

    if (b_colors)
    {
        snprintf(p_func_str, sizeof(p_func_str), "%c%s[%s] %c%s",
                 PL_TERM_CODE, PL_TERM_GRAY,
                 func,
                 PL_TERM_CODE, PL_TERM_RESET);
    }
    else
    {
        snprintf(p_func_str, sizeof(p_func_str), "[%s] ", func);
    }

    strncat(p_entry_str, p_func_str, PL_FUNC_LEN);
}

void
pl_write (pl_level_t level, const char* file, unsigned line,
                                const char* func, const char* p_fmt, ...)
{
    // Only write entry if there are registered appenders and the logger is
    // enabled
    if (0 == g_appender_count || !gb_enabled)
    {
        return;
    }

    // Ensure valid log level
    PL_ASSERT(level < PL_LEVEL_COUNT);

    for (pl_id_t i = 0; i < PL_MAX_APPENDERS; i++)
    {
        if (appender_enabled(i) &&
            gp_appenders[i].level <= level)
        {
            char p_entry_str[PL_ENTRY_LEN + 1]; // Ensure there is space for
                                                  // null char

            p_entry_str[0] = '\0'; // Ensure the entry is null terminated

            // Append a timestamp
            if (gp_appenders[i].b_timestamp)
            {
                append_timestamp(p_entry_str, gp_appenders[i].p_time_fmt);
            }

            // Append the logger level
            if (gp_appenders[i].b_level)
            {
                append_level(p_entry_str, level, gp_appenders[i].b_colors);
            }

            // Append the filename/line number
            if (gp_appenders[i].b_file)
            {
                append_file(p_entry_str, file, line, gp_appenders[i].b_colors);
            }

            // Append the function name
            if (gp_appenders[i].b_func)
            {
                append_func(p_entry_str, func, gp_appenders[i].b_colors);
            }

            // Append the log message
            char p_msg_str[PL_MSG_LEN];

            va_list args;
            va_start(args, p_fmt);
            vsnprintf(p_msg_str, sizeof(p_msg_str), p_fmt, args);
            va_end(args);

            strncat(p_entry_str, p_msg_str, PL_MSG_LEN);
            strcat(p_entry_str, "\n");

            // Locks the appender
            if (NULL != gp_appenders[i].p_lock)
            {
                gp_appenders[i].p_lock(true, gp_appenders[i].p_lock_udata);
            }

            gp_appenders[i].p_appender(p_entry_str, gp_appenders[i].p_udata);

            // Unlocks the appender
            if (NULL != gp_appenders[i].p_lock)
            {
                gp_appenders[i].p_lock(false, gp_appenders[i].p_lock_udata);
            }
        }
    }
}

#endif // PL_IMPLEMENTATION

// EoF
