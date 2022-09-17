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
    log_trace ("Test message: %d", 0);
    log_debug ("Test message: %d", 1);
    log_info  ("Test message: %d", 2);
    log_warn  ("Test message: %d", 3);
    log_error ("Test message: %d", 4);
    log_fatal ("Test message: %d", 5);
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    log_appender_t id1 = log_add_appender(appender1, LOG_LEVEL_TRACE, NULL);
    log_appender_t id2 = log_add_appender(appender2, LOG_LEVEL_INFO, NULL);

    log_set_level(id1, LOG_LEVEL_TRACE);
    log_set_level(id2, LOG_LEVEL_TRACE);

    printf("================== Both appenders ==================\n");

    log_all();

    printf("================== One appender ==================\n");

    log_disable_appender(id1);
    log_all();

    printf("================== Level Off ==================\n");

    log_display_level(id1, false);
    log_display_level(id2, false);

    log_all();

    printf("================== Level On/Set Level (INFO) ==================\n");

    log_enable_appender(id1);

    log_display_level(id1, true);
    log_display_level(id2, true);

    log_set_level(id1, LOG_LEVEL_INFO);
    log_set_level(id2, LOG_LEVEL_INFO);

    log_all();

    log_remove_appender(id2);

    printf("================== Timestamp ==================\n");

    id2 = log_add_appender(appender2, LOG_LEVEL_INFO, NULL);

    log_display_timestamp(id1, true);
    log_display_timestamp(id2, true);

    log_all();

    printf("================== File ==================\n");

    log_display_file(id1, true);
    log_display_file(id2, true);

    log_all();

    printf("================== Function ==================\n");

    log_display_function(id1, true);
    log_display_function(id2, true);

    log_all();

    return 0;
}

