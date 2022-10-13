#include "game.h"
#include "factories.h"
#include "files.h"
#include "hud.h"

#define PICO_ECS_IMPLEMENTATION
#include "pico_ecs.h"

#include <assert.h>
#include <curses.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#define PLAYER_KEY_LEFT  KEY_LEFT
#define PLAYER_KEY_RIGHT KEY_RIGHT
#define PLAYER_KEY_UP    KEY_UP
#define PLAYER_KEY_DOWN  KEY_DOWN
#define QUIT_KEY         27

#define MAP_PREFIX "./maps/map"

dim_t get_term_size()
{
    dim_t size;

    struct winsize winsize;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize);

    size.h = winsize.ws_row;
    size.w = winsize.ws_col;

    return size;
}

void term_refresh()
{
    erase();
    refresh();
}

void load_entities(game_t* game, level_t* level)
{
    game->player_id = make_player(game, level->player.x, level->player.y);

    for (int i = 0; i < level->npc_count; i++)
    {
        npc_t npc = level->npcs[i];

        switch (npc.type)
        {
            case MONSTER_SNAKE:
                make_snake(game, npc.pos.x, npc.pos.y);
                break;

            case MONSTER_GOBLIN:
                make_goblin(game, npc.pos.x, npc.pos.y);
                break;

            case MONSTER_ORC:
                make_orc(game, npc.pos.x, npc.pos.y);
                break;

            case MONSTER_KING:
                make_king(game, npc.pos.x, npc.pos.y);
                break;
        }
    }

    for (int i = 0; i < level->chest_count; i++)
    {
        chest_t chest = level->chests[i];
        make_chest(game, chest.pos.x, chest.pos.y);
    }
}

int level_count()
{
    int count = 0;

    while (true)
    {
        char path[PATH_MAX];
        snprintf(path, PATH_MAX, "%s%i.json", MAP_PREFIX, count);

        if (!file_exists(path))
            break;

        count++;
    }

    return count;
}

char* read_level(int level)
{
    char path[PATH_MAX];
    snprintf(path, PATH_MAX, "%s%i.json", MAP_PREFIX, level);

    char* buffer = read_file(path);
    assert(NULL != buffer);

    return buffer;
}

void load_levels(game_t* game)
{
    int count      = level_count();
    game->level_count = count;
    game->levels      = malloc(count * sizeof(level_t));
    game->maps        = malloc(count * sizeof(tilemap_t));

    for (int i = 0; i < count; i++)
    {
        char* buffer = read_level(i);
        game->levels[i] = load_level(buffer);
        game->maps[i] = create_tilemap(game, game->levels[i]);
        free(buffer);
    }
}

void setup_level(game_t* game)
{
    ecs_reset(game->ecs);

    int level = game->level;
    load_entities(game, game->levels[level]);

    game->map = game->maps[level];

    player_t* player = ecs_get(game->ecs, game->player_id, PLAYER_COMP);
    player->cmd = MOVE_NONE;
}

game_t* setup_game()
{
    srand(time(0));

    game_t* game = malloc(sizeof(game_t));
    memset(game, 0, sizeof(game_t));

    // Terminal dimensions
    game->term_size = get_term_size();

    // Create ECS
    game->ecs = ecs_new(128, NULL);

    // Load ECS components
    register_components(game);

    // Load systems
    register_systems(game);

    // Load all levels
    load_levels(game);

    // Initialize player

    // Initialize Curses
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    // Set up first level
    setup_level(game);

    draw_tilemap(game->maps[game->level]);
    ecs_update_system(game->ecs, DRAWABLE_SYS, 0.0);

    draw_player_msg(game, "Press any key to continue to start.");
    getch();

    term_refresh();

    return game;
}

void shutdown_game(game_t* game)
{
    for (int i = 0; i < game->level_count; i++)
    {
        free_level(game->levels[i]);
        free(game->maps[i]->tiles);
        free(game->maps[i]);
    }

    free(game->levels);
    free(game->maps);

    ecs_free(game->ecs);
    free(game);
    endwin();
}

void handle_input(game_t* game)
{
    player_t* player = ecs_get(game->ecs, game->player_id, PLAYER_COMP);

    int ch = getch();

    switch (ch)
    {
        case PLAYER_KEY_LEFT:
            player->cmd = MOVE_LEFT;
            break;

        case PLAYER_KEY_RIGHT:
            player->cmd = MOVE_RIGHT;
            break;

        case PLAYER_KEY_UP:
            player->cmd = MOVE_UP;
            break;

        case PLAYER_KEY_DOWN:
            player->cmd = MOVE_DOWN;
            break;

        case QUIT_KEY:
            game->quit = true;
            break;

        default:
            break;
    }
}

void level_over(game_t* game)
{
    term_refresh();

    if (game->level + 1 < game->level_count)
    {
        game->level++;
        draw_player_msg(game, "Level over!\n  Press any key to continue");
        setup_level(game);
        getch();
        term_refresh();
    }
    else
    {
        draw_player_msg(game, "Game over! The hero is victorious!\n  Press any key to quit.");
        game->quit = true;
        getch();
    }
}

void game_over(game_t* game)
{
    term_refresh();
    draw_player_msg(game, "The hero is dead! Game over!\n  Press any key to quit.");
    game->quit = true;
    getch();
}

int main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    game_t* game = setup_game();
    assert(game);

    while (!game->quit)
    {
        int level = game->level;

        draw_tilemap(game->maps[level]);

        ecs_ret_t code = ecs_update_systems(game->ecs, 0.0);

        if (LEVEL_OVER == code)
        {
            level_over(game);
        }
        else if (GAME_OVER == code)
        {
            game_over(game);
        }
        else
        {
            draw_hud(game);
            handle_input(game);
            term_refresh();
        }
    }

    shutdown_game(game);

    return 0;
}
