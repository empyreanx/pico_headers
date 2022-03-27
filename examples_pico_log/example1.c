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

    pl_id_t id = pl_add_appender(appender, PL_LEVEL_INFO, NULL);

    pl_display_timestamp(id, true);
    pl_display_file(id, true);
    pl_display_function(id, true);

    // Default log level is INFO

    pl_log_trace ("Test message: %d", 0);
    pl_log_debug ("Test message: %d", 1);
    pl_log_info  ("Test message: %d", 2);
    pl_log_warn  ("Test message: %d", 3);
    pl_log_error ("Test message: %d", 4);
    pl_log_fatal ("Test message: %d", 5);

    return 0;
}
