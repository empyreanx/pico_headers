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

#define PL_IMPLEMENTATION
#include "../pico_log.h"

#include <stdio.h>

static void appender1(const char* p_msg, void* p_user_data)
{
    (void)p_user_data;
    printf("Appender 1: %s", p_msg);
    fflush(stdout);
}

static void appender2(const char* p_msg, void* p_user_data)
{
    (void)p_user_data;
    printf("Appender 2: %s", p_msg);
    fflush(stdout);
}

static void log_all()
{
    pl_trace ("Test message: %d", 0);
    pl_debug ("Test message: %d", 1);
    pl_info  ("Test message: %d", 2);
    pl_warn  ("Test message: %d", 3);
    pl_error ("Test message: %d", 4);
    pl_fatal ("Test message: %d", 5);
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    pl_id_t id1 = pl_add_appender(appender1, PL_LEVEL_TRACE, NULL);
    pl_id_t id2 = pl_add_appender(appender2, PL_LEVEL_INFO, NULL);

    pl_set_level(id1, PL_LEVEL_TRACE);
    pl_set_level(id2, PL_LEVEL_TRACE);

    printf("================== Both appenders ==================\n");

    log_all();

    printf("================== One appender ==================\n");

    pl_disable_appender(id1);
    log_all();

    printf("================== Level Off ==================\n");

    pl_level_enabled(id1, false);
    pl_level_enabled(id2, false);

    log_all();

    printf("================== Level On/Set Level (INFO) ==================\n");

    pl_enable_appender(id1);

    pl_level_enabled(id1, true);
    pl_level_enabled(id2, true);

    pl_set_level(id1, PL_LEVEL_INFO);
    pl_set_level(id2, PL_LEVEL_INFO);

    log_all();

    pl_remove_appender(id2);

    printf("================== Timestamp ==================\n");

    id2 = pl_add_appender(appender2, PL_LEVEL_INFO, NULL);

    pl_timestamp_enabled(id1, true);
    pl_timestamp_enabled(id2, true);

    log_all();

    printf("================== File ==================\n");

    pl_file_enabled(id1, true);
    pl_file_enabled(id2, true);

    log_all();

    printf("================== Func ==================\n");

    pl_function_enabled(id1, true);
    pl_function_enabled(id2, true);

    log_all();

    return 0;
}

