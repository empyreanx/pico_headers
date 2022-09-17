#define PICO_LOG_IMPLEMENTATION
#include "../pico_log.h"

#include <stdio.h>

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    log_appender_t id = log_add_stream(stdout, LOG_LEVEL_TRACE);

    log_set_time_fmt(id, "%H:%M:%S");
    log_display_colors(id, true);
    log_display_timestamp(id, true);
    log_display_file(id, true);

    // Default log level is INFO

    log_trace ("Test message: %d", 0);
    log_debug ("Test message: %d", 1);
    log_info  ("Test message: %d", 2);
    log_warn  ("Test message: %d", 3);
    log_error ("Test message: %d", 4);
    log_fatal ("Test message: %d", 5);

    return 0;
}

