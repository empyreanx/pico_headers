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

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    pl_id_t id = pl_add_stream(stdout, PL_LEVEL_TRACE);

    pl_set_level(id, PL_LEVEL_TRACE);

    pl_set_time_fmt(id, "%H:%M:%S");
    pl_display_colors(id, true);
    pl_display_timestamp(id, true);
    pl_display_file(id, true);

    // Default log level is INFO

    pl_trace ("Test message: %d", 0);
    pl_debug ("Test message: %d", 1);
    pl_info  ("Test message: %d", 2);
    pl_warn  ("Test message: %d", 3);
    pl_error ("Test message: %d", 4);
    pl_fatal ("Test message: %d", 5);

    return 0;
}

