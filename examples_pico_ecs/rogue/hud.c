#include "hud.h"

#include "pico_ecs.h"

#include <curses.h>
#include <stdarg.h>
#include <stdio.h>

void draw_hud(game_t* game)
{
    stats_t* stats = ecs_get(game->ecs, game->player_id, STATS_COMP);

    dim_t term_size = game->term_size;

    int x = 2;
    int y = term_size.h - 5;

    mvprintw(y, x, "Health:  %i", stats->health);

    y++;
    mvprintw(y, x, "Attack:  %i", stats->attack);

    y++;
    mvprintw(y, x, "Defense: %i", stats->defense);
}

void draw_player_msg(game_t* game, const char* fmt, ...)
{
    char msg_str[1024];

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg_str, sizeof(msg_str), fmt, args);
    va_end(args);

    dim_t term_size = game->term_size;
    int x = 2;
    int y = term_size.h - 8;

    mvprintw(y, x, "%s", msg_str);
}

void draw_monster_msg(game_t* game, const char* fmt, ...)
{
    char msg_str[1024];

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg_str, sizeof(msg_str), fmt, args);
    va_end(args);

    dim_t term_size = game->term_size;
    int x = 2;
    int y = term_size.h - 7;

    mvprintw(y, x, "%s", msg_str);
}

void draw_debug_msg(game_t* game, const char* fmt, ...)
{
    char msg_str[1024];

    va_list args;
    va_start(args, fmt);
    vsnprintf(msg_str, sizeof(msg_str), fmt, args);
    va_end(args);

    dim_t term_size = game->term_size;
    int x = 2;
    int y = term_size.h - 6;

    mvprintw(y, x, "%s", msg_str);
}
