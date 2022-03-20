#define PICO_LOG_IMPLEMENTATION
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

    pl_display_level(id1, false);
    pl_display_level(id2, false);

    log_all();

    printf("================== Level On/Set Level (INFO) ==================\n");

    pl_enable_appender(id1);

    pl_display_level(id1, true);
    pl_display_level(id2, true);

    pl_set_level(id1, PL_LEVEL_INFO);
    pl_set_level(id2, PL_LEVEL_INFO);

    log_all();

    pl_remove_appender(id2);

    printf("================== Timestamp ==================\n");

    id2 = pl_add_appender(appender2, PL_LEVEL_INFO, NULL);

    pl_display_timestamp(id1, true);
    pl_display_timestamp(id2, true);

    log_all();

    printf("================== File ==================\n");

    pl_display_file(id1, true);
    pl_display_file(id2, true);

    log_all();

    printf("================== Function ==================\n");

    pl_display_function(id1, true);
    pl_display_function(id2, true);

    log_all();

    return 0;
}

