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

#define PLOG_IMPLEMENTATION
#include "../pico_log.h"

#include <stdio.h>

static void appender(const char* p_msg, void* p_user_data)
{
    (void)p_user_data;
    printf("%s", p_msg);
    fflush(stdout);
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    plog_id_t id = plog_add_appender(appender, PLOG_LEVEL_INFO, NULL);

    plog_timestamp_enabled(id, true);
    plog_file_enabled(id, true);
    plog_function_enabled(id, true);

    // Default log level is INFO

    plog_trace ("Test message: %d", 0);
    plog_debug ("Test message: %d", 1);
    plog_info  ("Test message: %d", 2);
    plog_warn  ("Test message: %d", 3);
    plog_error ("Test message: %d", 4);
    plog_fatal ("Test message: %d", 5);

    return 0;
}
