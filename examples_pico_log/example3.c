#define PICO_LOG_IMPLEMENTATION
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

