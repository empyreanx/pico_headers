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
pl_id_t pl_add_stream(FILE* stream, pl_level_t level);

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
void pl_colors_enabled(pl_id_t id, bool enabled);

/**
 * Turns timestamp reporting on/off for the specified appender.
 * NOTE: Off by default
 *
 * @param id The appender id
 * @param b_enabled On if true
 */
void pl_timestamp_enabled(pl_id_t id, bool enabled);

/**
 * Turns log level reporting on/off for the specified appender.
 * NOTE: On by default.
 *
 * @param id The appender id
 * @param b_enabled On if true
 */
void pl_level_enabled(pl_id_t id, bool enabled);

/**
 * Turns filename and line number reporting on/off for the specified appender.
 * NOTE: Off by default.
 *
 * @param id The appender id
 * @param b_enabled On if true
 */
void pl_file_enabled(pl_id_t id, bool enabled);

/**
 * Turns function reporting on/off for the specified appender.
 * NOTE: Off by default.
 *
 * @param id The appender id
 * @param b_enabled On if true
 */
void pl_function_enabled(pl_id_t id, bool enabled);

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
              const char* fmt, ...);


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

static bool   pl_initialized    = false; // True if logger is initialized
static bool   pl_enabled        = true;  // True if logger is enabled
static size_t pl_appender_count = 0;     // Number of appenders

/*
 * Logger level strings indexed by level ID (pl_level_t).
 */
static const char* const pl_level_str[] =
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
    pl_lock_fn     lock;
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

    for (size_t i = 0; pl_level_str[i]; i++)
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
pl_add_appender (pl_appender_fn appender_fp,
                 pl_level_t level,
                 void* p_udata)
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
            appender->udata       = p_udata;
            appender->level       = PL_LEVEL_INFO;
            appender->enabled     = true;
            appender->colors      = false;
            appender->level       = true;
            appender->timestamp   = false;
            appender->file        = false;
            appender->func        = false;
            appender->lock        = NULL;
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

void pl_set_lock(pl_id_t id, pl_lock_fn lock, void* udata)
{
    // Ensure lock function is initialized
    PL_ASSERT(NULL != lock);

    // Ensure appender is registered
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    pl_appenders[id].lock = lock;
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
pl_colors_enabled (pl_id_t id, bool enabled)
{
    // Initialize logger if neccesary
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    // Disable appender
    pl_appenders[id].colors = enabled;
}

void
pl_timestamp_enabled (pl_id_t id, bool enabled)
{
    // Initialize logger if neccesary
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    // Turn timestamp on
    pl_appenders[id].timestamp = enabled;
}

void
pl_level_enabled (pl_id_t id, bool enabled)
{
    // Initialize logger if neccesary
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    // Turn level reporting on
    pl_appenders[id].level = enabled;
}

void
pl_file_enabled (pl_id_t id, bool enabled)
{
    // Initialize logger if neccesary
    pl_try_init();

    // Ensure appender is registered
    PL_ASSERT(pl_appender_exists(id));

    // Turn file reporting on
    pl_appenders[id].file = enabled;
}

void
pl_function_enabled (pl_id_t id, bool enabled)
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
pl_time_str (const char* time_fmt, char* str, size_t len)
{
    time_t now = time(0);
    size_t ret = strftime(str, len, time_fmt, localtime(&now));

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
pl_append_file(char* entry_str, const char* file, unsigned line, bool colors)
{
    char file_str[PL_FILE_LEN];

    if (colors)
    {
        snprintf(file_str, sizeof(file_str), "%c%s%s:%u%c%s ",
                 PL_TERM_CODE, PL_TERM_GRAY,
                 file, line,
                 PL_TERM_CODE, PL_TERM_RESET);

    }
    else
    {
        snprintf(file_str, sizeof(file_str), "%s:%u ", file, line);
    }

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
pl_write (pl_level_t level, const char* file, unsigned line,
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

        if (pl_appenders[i].level <= level)
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
                pl_append_file(entry_str, file, line, appender->colors);
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
            if (NULL != appender->lock)
            {
                appender->lock(true, appender->lock_udata);
            }

            appender->appender_fp(entry_str, appender->udata);

            // Unlocks the appender
            if (NULL != appender->lock)
            {
                appender->lock(false, appender->lock_udata);
            }
        }
    }
}

#endif // PL_IMPLEMENTATION

// EoF
