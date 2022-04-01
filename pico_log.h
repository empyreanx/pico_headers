/**
    @file pico_log.h
    @brief A minimal and flexible logging framework written in C99.

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Features:
    ---------

    - Written in pure C99, and compatible with C++
    - Single header library for easy integration into any build system
    - Tiny memory and code footprint
    - Simple and minimalistic API
    - Flexible and extensible appender handling
    - Ability to set logging level (TRACE, DEBUG, INFO, WARN, ERROR, and FATAL)
    - Ability to toggle date/time, log level, filename/line, and function
    - reporting individually, on a per appender basis
    - Permissive licensing (zlib or public domain)

    Summary:
    --------

    This library is built around the notion of appenders. An appender writes log
    messages to a sink. It could be a file, a network connection, or stream
    (e.g. stdout).

    Once one or more appenders are registered, macros such as pl_info will send
    messages to the appenders.

    Output can be modified in a number of ways. The most important way to affect
    the output is to specify the log level (e.g. PL_LEVEL_INFO). If the log
    level is set PL_LEVEL_INFO, then messages sent to pl_trace or pl_debug will
    not be written whereas pl_info, pl_warn, pl_error, and pl_fatal will be.

    Output can also be modified to show or hide various metadata. These are
    date/time, log level, filename/line number, and calling function. They can
    be toggled using the pl_display* functions. There is also an option to
    enable color coded output.

    It is possible to synchronize appenders using `pl_set_lock`. This function
    accepts a function pointer that takes a boolean and user data as input. When
    this function pointer is passed true the lock is acquired and false to
    release the lock.

    Please see the examples for more details.

    Usage:
    ------

    To use this library in your project, add the following

    > #define PICO_LOG_IMPLEMENTATION
    > #include "pico_log.h"

    to a source file (once), then simply include the header normally.

    Constants:
    --------

    - PICO_LOG_MAX_APPENDERS (default: 16)
    - PICO_LOG_MAX_MSG_LENGTH (default: 1024)

    Must be defined before PICO_LOG_IMPLEMENTATION
*/

#ifndef PICO_LOG_H
#define PICO_LOG_H

#include <stdarg.h>  // ...
#include <stdbool.h> // bool, true, false
#include <stddef.h>  // NULL
#include <stdio.h>   // FILE

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief These codes allow different layers of granularity when logging. See
 * the documentation of the `pl_set_level` function for more information.
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
 * @brief Appender function definition. An appender writes a log entry to an
 * output stream. This could be the console, a file, a network connection, etc...
 */
typedef void (*pl_appender_fn)(const char* entry, void* udata);

/**
 *  @brief Lock function definition. This is called during pl_write_log. Adapted
 */
typedef void (*pl_lock_fn)(bool lock, void *udata);

/**
 * @brief Identifies a registered appender.
 */
typedef int pl_id_t;

/**
  * @brief Converts a string to the corresponding log level
  */
bool pl_str_level(const char* str, pl_level_t* level);

/**
 * @brief Enables logging. NOTE: Logging is enabled by default.
 */
void pl_enable(void);

/**
 * @brief Disables logging.
 */
void pl_disable(void);

/**
 * @brief Registers an appender
 *
 * Registers (adds appender to logger) and enables the specified appender. An
 * appender writes a log entry to an output stream. This could be a console,
 * a file, a network connection, etc...
 *
 * @param appender_fp Pointer to the appender function to register. An appender
 *                    function has the signature,
 *                    `void appender_func(const char* p_entry, void* p_udata)`
 *
 * @param level       The appender's log level
 *
 * @param udata       A pointer supplied to the appender function when writing
 *                    a log entry. This pointer is not modified by the logger.
 *                    If not required, pass in NULL for this parameter.
 *
 * @return            An identifier for the appender. This ID is valid until the
 *                    appender is unregistered.
 */
pl_id_t pl_add_appender(pl_appender_fn appender_fp,
                        pl_level_t level, void* udata);

/**
 * @brief Registers an output stream appender.
 *
 * @param stream The output stream to write to
 * @param level  The appender's log level
 *
 * @return       An identifier for the appender. This ID is valid until the
 *               appender is unregistered.
 */
pl_id_t pl_add_stream(FILE* stream, pl_level_t level);

/**
 * @brief Unregisters appender (removes the appender from the logger).
 *
 * @param id The appender to unregister
 */
void pl_remove_appender(pl_id_t id);

/**
 * @brief Enables the specified appender. NOTE: Appenders are enabled by default
 * after registration.
 *
 * @param id The appender to enable.
 */
void pl_enable_appender(pl_id_t id);

/**
 * @brief Disables the specified appender.
 *
 * @param id The appender to disable
 */
void pl_disable_appender(pl_id_t id);

/**
 * @brief Sets the locking function.
 */
void pl_set_lock(pl_id_t id, pl_lock_fn lock_fp, void* udata);

/**
 * @brief Sets the logging level

 * Sets the logging level. Only those messages of equal or higher priority
 * (severity) than this value will be logged.
 *
 * @param id    The appender to hold the lock
 * @param level The new appender logging threshold.
 */
void pl_set_level(pl_id_t id, pl_level_t level);

/**
 * @brief Set the appender timestamp.
 *
 * Sets the appender timestamp format according to:
 * https://man7.org/linux/man-pages/man3/strftime.3.html
 *
 * @param id The appender id
 * @param fmt The time format
 */
void pl_set_time_fmt(pl_id_t id, const char* fmt);

/**
 * @brief Turns colors ouput on or off for the specified appender.
 * NOTE: Off by default.
 *
 * @param id      The appender id
 * @param enabled On if true
 */
void pl_display_colors(pl_id_t id, bool enabled);

/**
 * @brief Turns timestamp reporting on/off for the specified appender.
 * NOTE: Off by default
 *
 * @param id      The appender id
 * @param enabled On if true
 */
void pl_display_timestamp(pl_id_t id, bool enabled);

/**
 * @brief Turns log level reporting on/off for the specified appender.
 * NOTE: On by default.
 *
 * @param id      The appender id
 * @param enabled On if true
 */
void pl_display_level(pl_id_t id, bool enabled);

/**
 * @brief Turns filename and line number reporting on/off for the specified
 * appender.
 * NOTE: Off by default.
 *
 * @param id      The appender id
 * @param enabled On if true
 */
void pl_display_file(pl_id_t id, bool enabled);

/**
 * @brief Turns function reporting on/off for the specified appender.
 * NOTE: Off by default.
 *
 * @param id      The appender id
 * @param enabled On if true
 */
void pl_display_function(pl_id_t id, bool enabled);

/**
 * @brief Logs a TRACE an INFO message
 *
 * Writes a TRACE level message to the log. Usage is similar to printf
 * (i.e. pl_trace(format, args...))
 */
#define pl_log_trace(...) \
        pl_write_log(PL_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief Logs a DEBUG message
 *
 * Writes a DEBUG level message to the log. Usage is similar to printf (i.e.
 * (i.e. pl_debug(format, args...))
 */
#define pl_log_debug(...) \
        pl_write_log(PL_LEVEL_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief Logs an INFO message
 *
 * Writes an INFO level message to the log. Usage is similar to printf
 * (i.e. pl_info(format, args...))
 */
#define pl_log_info(...) \
        pl_write_log(PL_LEVEL_INFO,  __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief Logs a WARN message
 *
 * Writes a WARN level message to the log. Usage is similar to printf (i.e.
 * (i.e. pl_warn(format, args...))
 */
#define pl_log_warn(...) \
        pl_write_log(PL_LEVEL_WARN,  __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief Logs an ERROR message
 *
 * Writes a ERROR level message to the log. Usage is similar to printf (i.e.
 * (i.e. pl_error(format, args...))
 */
#define pl_log_error(...) \
        pl_write_log(PL_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief Logs a FATAL message
 *
 * Writes a FATAL level message to the log.. Usage is similar to printf (i.e.
 * (i.e. pl_fatal(format, args...))
 */
#define pl_log_fatal(...) \
        pl_write_log(PL_LEVEL_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)


/**
 * WARNING: It is inadvisable to call this function directly. Use the macros
 * instead.
 */
void pl_write_log(pl_level_t level,
                  const char* file,
                  unsigned line,
                  const char* func,
                  const char* fmt, ...);


#ifdef __cplusplus
}
#endif

#endif // PICO_LOG_H

#ifdef PICO_LOG_IMPLEMENTATION

#include <time.h>
#include <string.h>

/*
 * Configuration constants/macros.
 */
#ifndef PICO_LOG_MAX_APPENDERS
#define PICO_LOG_MAX_APPENDERS 16
#endif

#ifndef PICO_LOG_MAX_MSG_LENGTH
#define PICO_LOG_MAX_MSG_LENGTH 1024
#endif

#ifdef NDEBUG
    #define PICO_LOG_ASSERT(expr) ((void)0)
#else
    #ifndef PICO_LOG_ASSERT
        #include <assert.h>
        #define PICO_LOG_ASSERT(expr) assert(expr)
    #endif
#endif

/*
 * Internal aliases
 */

#define PL_MAX_APPENDERS  PICO_LOG_MAX_APPENDERS
#define PL_MAX_MSG_LENGTH PICO_LOG_MAX_MSG_LENGTH
#define PL_ASSERT         PICO_LOG_ASSERT

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
#define PL_TIME_FMT     "%d/%m/%Y %H:%M:%S"

#define PL_TERM_CODE  0x1B
#define PL_TERM_RESET "[0m"
#define PL_TERM_GRAY  "[90m"

static bool pl_initialized    = false; // True if logger is initialized
static bool pl_enabled        = true;  // True if logger is enabled
static int  pl_appender_count = 0;     // Number of appenders

/*
 * Logger level strings indexed by level ID (pl_level_t).
 */
static const char* const pl_level_str[] =
{
    "TRACE",
    "DEBUG",
    "INFO ",
    "WARN ",
    "ERROR",
    "FATAL",
    0
};

/*
 * Logger level strings indexed by level ID (pl_level_t).
 */
static const char* const pl_level_str_formatted[] =
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
static const char* pl_level_color[] =
{
    "[94m", "[36m", "[32m", "[33m", "[31m", "[35m", NULL
};

/*
 * Appender pointer and metadata.
 */
typedef struct
{
    pl_appender_fn appender_fp;
    void*          udata;
    bool           enabled;
    pl_level_t     log_level;
    char           time_fmt[PL_TIME_FMT_LEN];
    bool           colors;
    bool           timestamp;
    bool           level;
    bool           file;
    bool           func;
    pl_lock_fn     lock_fp;
    void*          lock_udata;
} pl_appender_t;

/*
 * Array of appenders.
 */
static pl_appender_t pl_appenders[PL_MAX_APPENDERS];

/*
 * Initializes the logger provided it has not been initialized.
 */
static void
pl_try_init ()
{
    if (pl_initialized)
    {
        return;
    }

    for (int i = 0; i < PL_MAX_APPENDERS; i++)
    {
        pl_appenders[i].appender_fp = NULL;
    }

    pl_initialized = true;
}

static bool pl_appender_exists(pl_id_t id)
{
    return (id < PL_MAX_APPENDERS && NULL != pl_appenders[id].appender_fp);
}

static bool pl_appender_enabled(pl_id_t id)
{
    return pl_appender_exists(id) && pl_appenders[id].enabled;
}

bool pl_str_level(const char* str, pl_level_t* level)
{
    if (!level)
        return false;

    for (int i = 0; pl_level_str[i]; i++)
    {
        if (0 == strcmp(str, pl_level_str[i]))
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
    pl_enabled = true;
}

void
pl_disable (void)
{
    pl_enabled = false;
}

pl_id_t
pl_add_appender (pl_appender_fn appender_fp, pl_level_t level, void* udata)
{
    // Initialize logger if neccesary
    pl_try_init();

    // Check if there is space for a new appender.
    PL_ASSERT(pl_appender_count < PL_MAX_APPENDERS);

    // Ensure level is valid
    PL_ASSERT(level >= 0 && level < PL_LEVEL_COUNT);

    // Iterate through appender array and find an empty slot.
    for (int i = 0; i < PL_MAX_APPENDERS; i++)
    {
        if (NULL == pl_appenders[i].appender_fp)
        {
            pl_appender_t* appender = &pl_appenders[i];

            // Store and enable appender
            appender->appender_fp = appender_fp;
            appender->log_level   = level;
            appender->udata       = udata;
            appender->level       = PL_LEVEL_INFO;
            appender->enabled     = true;
            appender->colors      = false;
            appender->level       = true;
            appender->timestamp   = false;
            appender->file        = false;
            appender->func        = false;
            appender->lock_fp     = NULL;
            appender->lock_udata  = NULL;

            strncpy(appender->time_fmt, PL_TIME_FMT, PL_TIME_FMT_LEN);

            pl_appender_count++;

            return (pl_id_t)i;
        }
    }

    // This should never happen
    PL_ASSERT(false);
    return 0;
}

static void
pl_stream_appender (const char* entry, void* udata)
{
    FILE* stream = (FILE*)udata;
    fprintf(stream, "%s", entry);
    fflush(stream);
}

pl_id_t
pl_add_stream (FILE* stream, pl_level_t level)
{
    // Stream must not be NULL
    PL_ASSERT(NULL != stream);

    return pl_add_appender(pl_stream_appender, level, stream);
}

void
pl_remove_appender (pl_id_t id)
{
    // Initialize logger if neccesary
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    // Reset appender with given ID
    pl_appenders[id].appender_fp = NULL;

    pl_appender_count--;
}

void
pl_enable_appender (pl_id_t id)
{
    // Initialize logger if neccesary
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    // Enable appender
    pl_appenders[id].enabled = true;
}

void
pl_disable_appender (pl_id_t id)
{
    // Initialize logger if neccesary
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    // Disable appender
    pl_appenders[id].enabled = false;
}

void pl_set_lock(pl_id_t id, pl_lock_fn lock_fp, void* udata)
{
    // Ensure lock function is initialized
    PL_ASSERT(NULL != lock_fp);

    // Ensure appender is registered
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    pl_appenders[id].lock_fp = lock_fp;
    pl_appenders[id].lock_udata = udata;
}

void
pl_set_level (pl_id_t id, pl_level_t level)
{
    // Initialize logger if neccesary
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    // Ensure level is valid
    PL_ASSERT(level >= 0 && level < PL_LEVEL_COUNT);

    // Set the level
    pl_appenders[id].log_level = level;
}

void
pl_set_time_fmt (pl_id_t id, const char* fmt)
{
    // Initialize logger if neccesary
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    // Copy the time string
    strncpy(pl_appenders[id].time_fmt, fmt, PL_TIME_FMT_LEN);
}

void
pl_display_colors (pl_id_t id, bool enabled)
{
    // Initialize logger if neccesary
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    // Disable appender
    pl_appenders[id].colors = enabled;
}

void
pl_display_timestamp (pl_id_t id, bool enabled)
{
    // Initialize logger if neccesary
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    // Turn timestamp on
    pl_appenders[id].timestamp = enabled;
}

void
pl_display_level (pl_id_t id, bool enabled)
{
    // Initialize logger if neccesary
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    // Turn level reporting on
    pl_appenders[id].level = enabled;
}

void
pl_display_file (pl_id_t id, bool enabled)
{
    // Initialize logger if neccesary
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    // Turn file reporting on
    pl_appenders[id].file = enabled;
}

void
pl_display_function (pl_id_t id, bool enabled)
{
    // Initialize logger if neccesary
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    // Turn file reporting on
    pl_appenders[id].func = enabled;
}

/*
 * Formats the current time as as string.
 */
static char*
pl_time_str (const char* time_fmt, char* str, int len)
{
    time_t now = time(0);
    int ret = strftime(str, len, time_fmt, localtime(&now));

    PL_ASSERT(ret > 0);

    return str;
}

static void
pl_append_timestamp (char* entry_str, const char* time_fmt)
{
    char time_str[PL_TIMESTAMP_LEN + 1];
    char tmp_str[PL_TIMESTAMP_LEN + 1];

    snprintf(time_str, PL_TIMESTAMP_LEN, "%s ",
             pl_time_str(time_fmt, tmp_str, PL_TIMESTAMP_LEN));

    strncat(entry_str, time_str, PL_TIMESTAMP_LEN);
}

static void
pl_append_level (char* entry_str, pl_level_t level, bool colors)
{
    char level_str[PL_LEVEL_LEN];

    if (colors)
    {
        snprintf(level_str, sizeof(level_str), "%c%s%s %c%s",
        PL_TERM_CODE, pl_level_color[level],
        pl_level_str_formatted[level],
        PL_TERM_CODE, PL_TERM_RESET);
    }
    else
    {
        snprintf(level_str, sizeof(level_str), "%s ", pl_level_str[level]);
    }

    strncat(entry_str, level_str, PL_LEVEL_LEN);
}

static void
pl_append_file(char* entry_str, const char* file, unsigned line)
{
    char file_str[PL_FILE_LEN];
    snprintf(file_str, sizeof(file_str), "[%s:%u] ", file, line);
    strncat(entry_str, file_str, PL_FILE_LEN);
}

static void
pl_append_func(char* entry_str, const char* func, bool colors)
{
   char func_str[PL_FUNC_LEN];

    if (colors)
    {
        snprintf(func_str, sizeof(func_str), "%c%s[%s] %c%s",
                 PL_TERM_CODE, PL_TERM_GRAY,
                 func,
                 PL_TERM_CODE, PL_TERM_RESET);
    }
    else
    {
        snprintf(func_str, sizeof(func_str), "[%s] ", func);
    }

    strncat(entry_str, func_str, PL_FUNC_LEN);
}

void
pl_write_log (pl_level_t level, const char* file, unsigned line,
                            const char* func, const char* fmt, ...)
{
    // Only write entry if there are registered appenders and the logger is
    // enabled
    if (0 == pl_appender_count || !pl_enabled)
    {
        return;
    }

    // Ensure valid log level
    PL_ASSERT(level < PL_LEVEL_COUNT);

    for (pl_id_t i = 0; i < PL_MAX_APPENDERS; i++)
    {
        pl_appender_t* appender = &pl_appenders[i];

        if (!pl_appender_enabled(i))
            continue;

        if (pl_appenders[i].log_level <= level)
        {
            char entry_str[PL_ENTRY_LEN + 1]; // Ensure there is space for
                                              // null char

            entry_str[0] = '\0'; // Ensure the entry is null terminated

            // Append a timestamp
            if (appender->timestamp)
            {
                pl_append_timestamp(entry_str, appender->time_fmt);
            }

            // Append the logger level
            if (appender->level)
            {
                pl_append_level(entry_str, level, appender->colors);
            }

            // Append the filename/line number
            if (appender->file)
            {
                pl_append_file(entry_str, file, line);
            }

            // Append the function name
            if (appender->func)
            {
                pl_append_func(entry_str, func, appender->colors);
            }

            // Append the log message
            char msg_str[PL_MSG_LEN];

            va_list args;
            va_start(args, fmt);
            vsnprintf(msg_str, sizeof(msg_str), fmt, args);
            va_end(args);

            strncat(entry_str, msg_str, PL_MSG_LEN);
            strcat(entry_str, "\n");

            // Locks the appender
            if (NULL != appender->lock_fp)
            {
                appender->lock_fp(true, appender->lock_udata);
            }

            appender->appender_fp(entry_str, appender->udata);

            // Unlocks the appender
            if (NULL != appender->lock_fp)
            {
                appender->lock_fp(false, appender->lock_udata);
            }
        }
    }
}

#endif // PICO_LOG_IMPLEMENTATION


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
