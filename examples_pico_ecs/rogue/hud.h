#ifndef HUD_H
#define HUD_H

#include "game.h"

void draw_hud(game_t* game);
void draw_player_msg(game_t* game, const char* fmt, ...);
void draw_monster_msg(game_t* game, const char* fmt, ...);
void draw_debug_msg(game_t* game, const char* fmt, ...);

#endif
