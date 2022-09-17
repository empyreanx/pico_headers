#define PICO_LOG_IMPLEMENTATION
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

    log_appender_t id = log_add_appender(appender, LOG_LEVEL_INFO, NULL);

    log_display_timestamp(id, true);
    log_display_file(id, true);
    log_display_function(id, true);

    // Default log level is INFO

    log_trace ("Test message: %d", 0);
    log_debug ("Test message: %d", 1);
    log_info  ("Test message: %d", 2);
    log_warn  ("Test message: %d", 3);
    log_error ("Test message: %d", 4);
    log_fatal ("Test message: %d", 5);

    return 0;
}
