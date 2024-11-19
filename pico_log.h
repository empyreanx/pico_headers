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

    Once one or more appenders are registered, macros such as log_info will send
    messages to the appenders.

    Output can be modified in a number of ways. The most important way to affect
    the output is to specify the log level (e.g. LOG_LEVEL_INFO). If the log
    level is set LOG_LEVEL_INFO, then messages sent to log_trace or log_debug
    will not be written whereas log_info, log_warn, log_error, and log_fatal
    will have no effect.

    Output can also be modified to show or hide various metadata. These are
    date/time, log level, filename/line number, and calling function. They can
    be toggled using the log_display* functions. There is also an option to
    enable color coded output.

    It is possible to synchronize appenders using `log_set_lock`. This function
    accepts a function pointer that takes a boolean and user data as input. If
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
 * the documentation of the `log_set_level` function for more information.
 */
typedef enum
{
    LOG_LEVEL_TRACE = 0,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
    LOG_LEVEL_COUNT
} log_level_t;

/**
 * @brief Appender function definition. An appender writes a log entry to an
 * output stream. This could be the console, a file, a network connection, etc...
 */
typedef void (*log_appender_fn)(const char* entry, void* udata);

/**
 * @brief Lock function definition. This is called during log_write.
 *
 * @param lock  True to lock, false to release the lock
 * @param udata Data obtained from the corresponding `log_set_lock` function
 */
typedef void (*log_appender_lock_fn)(bool lock, void *udata);

/**
 * @brief Identifies a registered appender.
 */
typedef int log_appender_t;

/**
  * @brief Converts a string to the corresponding log level
  */
bool log_str_to_level(const char* str, log_level_t* level);

/**
 * @brief Enables logging. NOTE: Logging is enabled by default.
 */
void log_enable(void);

/**
 * @brief Disables logging.
 */
void log_disable(void);

/**
 * @brief Registers an appender
 *
 * Adds appender to logger and enables the specified appender. An appender
 * writes a log entry to an output stream. This could be a console, a file, a
 * network connection, etc...
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
log_appender_t log_add_appender(log_appender_fn appender_fp,
                                log_level_t level,
                                void* udata);

/**
 * @brief Registers an output stream appender.
 *
 * @param stream The output stream to write to
 * @param level  The appender's log level
 *
 * @return       An identifier for the appender. This ID is valid until the
 *               appender is unregistered.
 */
log_appender_t log_add_stream(FILE* stream, log_level_t level);

/**
 * @brief Unregisters appender (removes the appender from the logger).
 *
 * @param id The appender to unregister
 */
void log_remove_appender(log_appender_t id);

/**
 * @brief Enables the specified appender. NOTE: Appenders are enabled by default
 * after registration.
 *
 * @param id The appender to enable.
 */
void log_enable_appender(log_appender_t id);

/**
 * @brief Disables the specified appender.
 *
 * @param id The appender to disable
 */
void log_disable_appender(log_appender_t id);

/**
 * @brief Sets the lock function for a given appender

 ^ @param lock_fp The lock function pointer
 * @param id      The appender to hold the lock
 */
void log_set_lock(log_appender_t id, log_appender_lock_fn lock_fp, void* udata);

/**
 * @brief Sets the logging level

 * Sets the logging level. Only those messages of equal or higher priority
 * (severity) than this value will be logged.
 *
 * @param id    The appender
 * @param level The new appender logging threshold.
 */
void log_set_level(log_appender_t id, log_level_t level);

/**
 * @brief Set the appender timestamp.
 *
 * Sets the appender timestamp format according to:
 * https://man7.org/linux/man-pages/man3/strftime.3.html
 *
 * @param id The appender id
 * @param fmt The time format
 */
void log_set_time_fmt(log_appender_t id, const char* fmt);

/**
 * @brief Turns colors ouput on or off for the specified appender.
 * NOTE: Off by default.
 *
 * @param id      The appender id
 * @param enabled On if true
 */
void log_display_colors(log_appender_t id, bool enabled);

/**
 * @brief Turns timestamp reporting on/off for the specified appender.
 * NOTE: Off by default
 *
 * @param id      The appender id
 * @param enabled On if true
 */
void log_display_timestamp(log_appender_t id, bool enabled);

/**
 * @brief Turns log level reporting on/off for the specified appender.
 * NOTE: On by default.
 *
 * @param id      The appender id
 * @param enabled On if true
 */
void log_display_level(log_appender_t id, bool enabled);

/**
 * @brief Turns filename and line number reporting on/off for the specified
 * appender.
 * NOTE: Off by default.
 *
 * @param id      The appender id
 * @param enabled On if true
 */
void log_display_file(log_appender_t id, bool enabled);

/**
 * @brief Turns function reporting on/off for the specified appender.
 * NOTE: Off by default.
 *
 * @param id      The appender id
 * @param enabled On if true
 */
void log_display_function(log_appender_t id, bool enabled);

/**
 * @brief Logs a TRACE an INFO message
 *
 * Writes a TRACE level message to the log. Usage is similar to printf
 * (i.e. log_trace(format, args...))
 */
#define log_trace(...) \
        log_write(LOG_LEVEL_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief Logs a DEBUG message
 *
 * Writes a DEBUG level message to the log. Usage is similar to printf (i.e.
 * (i.e. log_debug(format, args...))
 */
#define log_debug(...) \
        log_write(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief Logs an INFO message
 *
 * Writes an INFO level message to the log. Usage is similar to printf
 * (i.e. log_info(format, args...))
 */
#define log_info(...) \
        log_write(LOG_LEVEL_INFO,  __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief Logs a WARN message
 *
 * Writes a WARN level message to the log. Usage is similar to printf (i.e.
 * (i.e. log_warn(format, args...))
 */
#define log_warn(...) \
        log_write(LOG_LEVEL_WARN,  __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief Logs an ERROR message
 *
 * Writes a ERROR level message to the log. Usage is similar to printf (i.e.
 * (i.e. log_error(format, args...))
 */
#define log_error(...) \
        log_write(LOG_LEVEL_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)

/**
 * @brief Logs a FATAL message
 *
 * Writes a FATAL level message to the log.. Usage is similar to printf (i.e.
 * (i.e. log_fatal(format, args...))
 */
#define log_fatal(...) \
        log_write(LOG_LEVEL_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)


/**
 * WARNING: It is inadvisable to call this function directly. Use the macros
 * instead.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 5, 6)))
#endif
void log_write(log_level_t level,
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

#define LOG_MAX_APPENDERS  PICO_LOG_MAX_APPENDERS
#define LOG_MAX_MSG_LENGTH PICO_LOG_MAX_MSG_LENGTH
#define LOG_ASSERT         PICO_LOG_ASSERT

/*
 * Log entry component maximum sizes. These have been chosen to be overly
 * generous powers of 2 for the sake of safety and simplicity.
 */

#define LOG_TIMESTAMP_LEN 64
#define LOG_LEVEL_LEN     32
#define LOG_FILE_LEN      512
#define LOG_FUNC_LEN      32
#define LOG_MSG_LEN       LOG_MAX_MSG_LENGTH
#define LOG_BREAK_LEN     1

#define LOG_ENTRY_LEN (LOG_TIMESTAMP_LEN  + \
                       LOG_LEVEL_LEN      + \
                       LOG_FILE_LEN       + \
                       LOG_FUNC_LEN       + \
                       LOG_MSG_LEN        + \
                       LOG_BREAK_LEN)

#define LOG_TIME_FMT_LEN 32
#define LOG_TIME_FMT     "%d/%m/%Y %H:%M:%S"

#define LOG_TERM_CODE  0x1B
#define LOG_TERM_RESET "[0m"
#define LOG_TERM_GRAY  "[90m"

static bool log_initialized    = false; // True if logger is initialized
static bool log_enabled        = true;  // True if logger is enabled
static int  log_appender_count = 0;     // Number of appenders

/*
 * Logger level strings indexed by level ID (log_level_t).
 */
static const char* const log_level_str[] =
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
 * Logger level strings indexed by level ID (log_level_t).
 */
static const char* const log_level_str_formatted[] =
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
static const char* log_level_color[] =
{
    "[94m", "[36m", "[32m", "[33m", "[31m", "[35m", NULL
};

/*
 * Appender pointer and metadata.
 */
typedef struct
{
    log_appender_fn      appender_fp;
    void*                udata;
    log_appender_lock_fn lock_fp;
    void*                lock_udata;
    bool                 enabled;
    log_level_t          log_level;
    char                 time_fmt[LOG_TIME_FMT_LEN];
    bool                 colors;
    bool                 timestamp;
    bool                 level;
    bool                 file;
    bool                 func;
} log_appender_data_t;

/*
 * Array of appenders.
 */
static log_appender_data_t log_appenders[LOG_MAX_APPENDERS];

/*
 * Initializes the logger provided it has not been initialized.
 */
static void
log_try_init (void)
{
    if (log_initialized)
    {
        return;
    }

    for (int i = 0; i < LOG_MAX_APPENDERS; i++)
    {
        log_appenders[i].appender_fp = NULL;
    }

    log_initialized = true;
}

static bool log_appender_exists(log_appender_t id)
{
    log_try_init();

    return (id < LOG_MAX_APPENDERS && NULL != log_appenders[id].appender_fp);
}

static bool log_appender_enabled(log_appender_t id)
{
    log_try_init();

    return log_appender_exists(id) && log_appenders[id].enabled;
}

bool log_str_to_level(const char* str, log_level_t* level)
{
    if (!level)
        return false;

    for (int i = 0; log_level_str[i]; i++)
    {
        if (0 == strcmp(str, log_level_str[i]))
        {
            *level = (log_level_t)i;
            return true;
        }
    }

    return false;
}

void
log_enable (void)
{
    log_enabled = true;
}

void
log_disable (void)
{
    log_enabled = false;
}

log_appender_t
log_add_appender (log_appender_fn appender_fp, log_level_t level, void* udata)
{
    // Initialize logger if neccesary
    log_try_init();

    // Check if there is space for a new appender.
    LOG_ASSERT(log_appender_count < LOG_MAX_APPENDERS);

    // Ensure level is valid
    LOG_ASSERT(level >= 0 && level < LOG_LEVEL_COUNT);

    // Iterate through appender array and find an empty slot.
    for (int i = 0; i < LOG_MAX_APPENDERS; i++)
    {
        if (NULL == log_appenders[i].appender_fp)
        {
            log_appender_data_t* appender = &log_appenders[i];

            // Store and enable appender
            appender->appender_fp = appender_fp;
            appender->log_level   = level;
            appender->udata       = udata;
            appender->level       = LOG_LEVEL_INFO;
            appender->enabled     = true;
            appender->colors      = false;
            appender->level       = true;
            appender->timestamp   = false;
            appender->file        = false;
            appender->func        = false;
            appender->lock_fp     = NULL;
            appender->lock_udata  = NULL;

            strncpy(appender->time_fmt, LOG_TIME_FMT, LOG_TIME_FMT_LEN);

            log_appender_count++;

            return (log_appender_t)i;
        }
    }

    // This should never happen
    LOG_ASSERT(false);
    return 0;
}

static void
log_stream_appender (const char* entry, void* udata)
{
    FILE* stream = (FILE*)udata;
    fprintf(stream, "%s", entry);
    fflush(stream);
}

log_appender_t
log_add_stream (FILE* stream, log_level_t level)
{
    // Stream must not be NULL
    LOG_ASSERT(NULL != stream);

    return log_add_appender(log_stream_appender, level, stream);
}

void
log_remove_appender (log_appender_t id)
{
    // Initialize logger if neccesary
    log_try_init();

    // Ensure appender is registered
    LOG_ASSERT(log_appender_exists(id));

    // Reset appender with given ID
    log_appenders[id].appender_fp = NULL;

    log_appender_count--;
}

void
log_enable_appender (log_appender_t id)
{
    // Initialize logger if neccesary
    log_try_init();

    // Ensure appender is registered
    LOG_ASSERT(log_appender_exists(id));

    // Enable appender
    log_appenders[id].enabled = true;
}

void
log_disable_appender (log_appender_t id)
{
    // Initialize logger if neccesary
    log_try_init();

    // Ensure appender is registered
    LOG_ASSERT(log_appender_exists(id));

    // Disable appender
    log_appenders[id].enabled = false;
}

void
log_set_lock (log_appender_t id, log_appender_lock_fn lock_fp, void* udata)
{
    // Ensure lock function is initialized
    LOG_ASSERT(NULL != lock_fp);

    // Ensure appender is registered
    log_try_init();

    // Ensure appender is registered
    LOG_ASSERT(log_appender_exists(id));

    log_appenders[id].lock_fp = lock_fp;
    log_appenders[id].lock_udata = udata;
}

void
log_set_level (log_appender_t id, log_level_t level)
{
    // Initialize logger if neccesary
    log_try_init();

    // Ensure appender is registered
    LOG_ASSERT(log_appender_exists(id));

    // Ensure level is valid
    LOG_ASSERT(level >= 0 && level < LOG_LEVEL_COUNT);

    // Set the level
    log_appenders[id].log_level = level;
}

void
log_set_time_fmt (log_appender_t id, const char* fmt)
{
    // Initialize logger if neccesary
    log_try_init();

    // Ensure appender is registered
    LOG_ASSERT(log_appender_exists(id));

    // Copy the time string
    strncpy(log_appenders[id].time_fmt, fmt, LOG_TIME_FMT_LEN);
}

void
log_display_colors (log_appender_t id, bool enabled)
{
    // Initialize logger if neccesary
    log_try_init();

    // Ensure appender is registered
    LOG_ASSERT(log_appender_exists(id));

    // Disable appender
    log_appenders[id].colors = enabled;
}

void
log_display_timestamp (log_appender_t id, bool enabled)
{
    // Initialize logger if neccesary
    log_try_init();

    // Ensure appender is registered
    LOG_ASSERT(log_appender_exists(id));

    // Turn timestamp on
    log_appenders[id].timestamp = enabled;
}

void
log_display_level (log_appender_t id, bool enabled)
{
    // Initialize logger if neccesary
    log_try_init();

    // Ensure appender is registered
    LOG_ASSERT(log_appender_exists(id));

    // Turn level reporting on
    log_appenders[id].level = enabled;
}

void
log_display_file (log_appender_t id, bool enabled)
{
    // Initialize logger if neccesary
    log_try_init();

    // Ensure appender is registered
    LOG_ASSERT(log_appender_exists(id));

    // Turn file reporting on
    log_appenders[id].file = enabled;
}

void
log_display_function (log_appender_t id, bool enabled)
{
    // Initialize logger if neccesary
    log_try_init();

    // Ensure appender is registered
    LOG_ASSERT(log_appender_exists(id));

    // Turn file reporting on
    log_appenders[id].func = enabled;
}

/*
 * Formats the current time as as string.
 */
static char*
log_time_str (const char* time_fmt, char* str, size_t len)
{
    time_t now = time(0);
    size_t ret = strftime(str, len, time_fmt, localtime(&now));

    LOG_ASSERT(ret > 0);

    return str;
}

static void
log_append_timestamp (char* entry_str, const char* time_fmt)
{
    char time_str[LOG_TIMESTAMP_LEN + 1];
    char tmp_str[LOG_TIMESTAMP_LEN];

    snprintf(time_str, sizeof(time_str), "%s ",
             log_time_str(time_fmt, tmp_str, sizeof(tmp_str)));

    strncat(entry_str, time_str, LOG_TIMESTAMP_LEN);
}

static void
log_append_level (char* entry_str, log_level_t level, bool colors)
{
    char level_str[LOG_LEVEL_LEN];

    if (colors)
    {
        snprintf(level_str, sizeof(level_str), "%c%s%s %c%s",
        LOG_TERM_CODE, log_level_color[level],
        log_level_str_formatted[level],
        LOG_TERM_CODE, LOG_TERM_RESET);
    }
    else
    {
        snprintf(level_str, sizeof(level_str), "%s ", log_level_str_formatted[level]);
    }

    strncat(entry_str, level_str, LOG_LEVEL_LEN);
}

static void
log_append_file (char* entry_str, const char* file, unsigned line)
{
    char file_str[LOG_FILE_LEN];
    snprintf(file_str, sizeof(file_str), "[%s:%u] ", file, line);
    strncat(entry_str, file_str, LOG_FILE_LEN);
}

static void
log_append_func (char* entry_str, const char* func, bool colors)
{
   char func_str[LOG_FUNC_LEN];

    if (colors)
    {
        snprintf(func_str, sizeof(func_str), "%c%s[%s] %c%s",
                 LOG_TERM_CODE, LOG_TERM_GRAY,
                 func,
                 LOG_TERM_CODE, LOG_TERM_RESET);
    }
    else
    {
        snprintf(func_str, sizeof(func_str), "[%s] ", func);
    }

    strncat(entry_str, func_str, LOG_FUNC_LEN);
}

void
log_write (log_level_t level, const char* file, unsigned line,
                              const char* func, const char* fmt, ...)
{
    // Ensure logger is initialized
    log_try_init();

    // Only write entry if there are registered appenders and the logger is
    // enabled
    if (0 == log_appender_count || !log_enabled)
    {
        return;
    }

    // Ensure valid log level
    LOG_ASSERT(level < LOG_LEVEL_COUNT);

    for (log_appender_t i = 0; i < LOG_MAX_APPENDERS; i++)
    {
        log_appender_data_t* appender = &log_appenders[i];

        if (!log_appender_enabled(i))
            continue;

        if (log_appenders[i].log_level <= level)
        {
            char entry_str[LOG_ENTRY_LEN + 1]; // Ensure there is space for
                                              // null char

            entry_str[0] = '\0'; // Ensure the entry is null terminated

            // Append a timestamp
            if (appender->timestamp)
            {
                log_append_timestamp(entry_str, appender->time_fmt);
            }

            // Append the logger level
            if (appender->level)
            {
                log_append_level(entry_str, level, appender->colors);
            }

            // Append the filename/line number
            if (appender->file)
            {
                log_append_file(entry_str, file, line);
            }

            // Append the function name
            if (appender->func)
            {
                log_append_func(entry_str, func, appender->colors);
            }

            // Append the log message
            char msg_str[LOG_MSG_LEN];

            va_list args;
            va_start(args, fmt);
            vsnprintf(msg_str, sizeof(msg_str), fmt, args);
            va_end(args);

            strncat(entry_str, msg_str, LOG_MSG_LEN);
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
