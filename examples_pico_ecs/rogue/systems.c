#include "components.h"
#include "game.h"
#include "hud.h"
#include "tilemaps.h"

#include "pico_ecs.h"

#include <assert.h>
#include <curses.h>
#include <stdbool.h>
#include <stdio.h>

ecs_id_t PLAYER_SYS;
ecs_id_t MONSTER_SYS;
ecs_id_t CHEST_SYS;
ecs_id_t DRAWABLE_SYS;

bool get_entity_at(ecs_t* ecs,
                   pos_t pos,
                   ecs_id_t entities[],
                   int entity_count,
                   ecs_id_t* id)
{
    for (int i = 0; i < entity_count; i++)
    {
        pos_t* entity_pos = ecs_get(ecs, entities[i], POS_COMP);

        if (pos_equal(&pos, entity_pos))
        {
            if (NULL != id)
                *id = entities[i];

            return true;
        }
    }

    return false;
}

bool perform_attack(game_t* game, ecs_id_t attacker_id, ecs_id_t defender_id)
{
    ecs_t* ecs = game->ecs;

    bool is_player = ecs_has(ecs, attacker_id, PLAYER_COMP);

    stats_t* attacker = ecs_get(ecs, attacker_id, STATS_COMP);
    stats_t* defender = ecs_get(ecs, defender_id, STATS_COMP);

    int damage  = random_int(0, attacker->attack);
    int defense = defender->defense;
    damage = max(damage - defense, 0);
    defender->health -= damage;

    if (defender->health <= 0)
    {
        if (is_player)
            draw_player_msg(game, "Hero hits for %i damage and kills monster", damage);
        else
            draw_monster_msg(game, "Monster hits for %i damage and kills hero", damage);

        return true;
    }
    else
    {
        if (is_player)
            draw_player_msg(game, "Hero hits for %i damage", damage);
        else
            draw_monster_msg(game, "Monster hits for %i damage", damage);

        return false;
    }
}

pos_t get_move_offset(move_t dir)
{
    pos_t offset;

    switch (dir)
    {
        case MOVE_LEFT:
            offset.x = -1;
            offset.y = 0;
            break;

        case MOVE_RIGHT:
            offset.x = 1;
            offset.y = 0;
            break;

        case MOVE_UP:
            offset.x = 0;
            offset.y = -1;
            break;

        case MOVE_DOWN:
            offset.x = 0;
            offset.y = 1;
            break;

        case MOVE_NONE:
            offset.x = 0;
            offset.y = 0;
            break;

        default:
            assert(false);
            break;
    }

    return offset;
}


ecs_ret_t player_sys(ecs_t* ecs,
                     ecs_id_t* entities,
                     int entity_count,
                     ecs_dt_t dt,
                     void* udata)
{
    (void)dt;

    game_t* game = udata;

    int level = game->level;
    tilemap_t* map = game->maps[level];

    player_t* player = ecs_get(ecs, game->player_id, PLAYER_COMP);

    pos_t* player_pos = ecs_get(ecs, game->player_id, POS_COMP);

    pos_t offset = get_move_offset(player->cmd);
    pos_t pos = pos_add(player_pos, &offset);

    ecs_id_t target_id;

    if (get_entity_at(game->ecs, pos, entities, entity_count, &target_id))
    {
        if (ecs_has(ecs, target_id, MONSTER_COMP))
        {
            if (perform_attack(game, game->player_id, target_id))
                ecs_queue_destroy(ecs, target_id);
        }

        return 0;
    }

    if (!collides_with_tile(map, pos))
        *player_pos = pos;

    return 0;
}

bool monster_try_move(ecs_t* ecs,
                      game_t* game,
                      tilemap_t* map,
                      ecs_id_t monster_id,
                      pos_t pos,
                      ecs_id_t entities[],
                      int entity_count)
{
    pos_t* monster_pos = ecs_get(ecs, monster_id, POS_COMP);

    if (collides_with_tile(map, pos))
        return false;

    if (get_entity_at(ecs, pos, entities, entity_count, NULL))
        return false;

    pos_t* player_pos = ecs_get(ecs, game->player_id, POS_COMP);

    if (pos_equal(&pos, player_pos))
        return false;

    *monster_pos = pos;

    return true;
}

void monster_do_wander(ecs_t* ecs,
                       game_t* game,
                       tilemap_t* map,
                       ecs_id_t player_id,
                       ecs_id_t monster_id,
                       ecs_id_t entities[],
                       int entity_count)
{
    pos_t* player_pos = ecs_get(ecs, player_id, POS_COMP);

    monster_t* monster = ecs_get(ecs, monster_id, MONSTER_COMP);
    pos_t* monster_pos = ecs_get(ecs, monster_id, POS_COMP);

    pos_t  move_pos = random_move(monster_pos);
    monster_try_move(ecs, game, map, monster_id, move_pos, entities, entity_count);

    if (distance(player_pos, monster_pos) <= 4)
    {
        draw_monster_msg(game, "Monster pursues!\n");
        monster->state = MONSTER_PURSUE;
        return;
    }
}

void monster_do_pursue(ecs_t* ecs,
                       game_t* game,
                       tilemap_t* map,
                       ecs_id_t player_id,
                       ecs_id_t monster_id,
                       ecs_id_t entities[],
                       int entity_count)
{
    pos_t* player_pos = ecs_get(ecs, player_id, POS_COMP);
    monster_t* monster = ecs_get(ecs, monster_id, MONSTER_COMP);
    pos_t* monster_pos = ecs_get(ecs, monster_id, POS_COMP);

    if (distance(player_pos, monster_pos) <= 1)
    {
        draw_monster_msg(game, "Monster attacks!\n");
        monster->state = MONSTER_ATTACK;
        return;
    }

    pos_t move_pos = move_toward(map, monster_pos, player_pos);
    monster_try_move(ecs, game, map, monster_id, move_pos, entities, entity_count);
}

void monster_do_attack(game_t* game, ecs_id_t player_id, ecs_id_t monster_id)
{
    pos_t* player_pos = ecs_get(game->ecs, player_id, POS_COMP);
    monster_t* monster = ecs_get(game->ecs, monster_id, MONSTER_COMP);
    pos_t* monster_pos = ecs_get(game->ecs, monster_id, POS_COMP);
    stats_t* monster_stats = ecs_get(game->ecs, monster_id, STATS_COMP);

    if (monster_stats->health < 3)
    {
        draw_monster_msg(game, "Monster flees...\n");
        monster->state = MONSTER_FLEE;
        return;
    }

    if (distance(player_pos, monster_pos) > 1)
    {
        monster->state = MONSTER_PURSUE;
        return;
    }

    if (perform_attack(game, monster_id, player_id))
    {
        game->player_dead = true;
    }
}

void monster_do_flee(ecs_t* ecs,
                     game_t* game,
                     tilemap_t* map,
                     ecs_id_t player_id,
                     ecs_id_t monster_id,
                     ecs_id_t entities[],
                     int entity_count)
{
    monster_t* monster = ecs_get(ecs, monster_id, MONSTER_COMP);
    pos_t* monster_pos = ecs_get(ecs, monster_id, POS_COMP);
    pos_t* player_pos = ecs_get(ecs, player_id, POS_COMP);

    if (distance(player_pos, monster_pos) > 3)
    {
        monster->state = MONSTER_WANDER;
        return;
    }

    pos_t move_pos = move_away(map, monster_pos, player_pos);
    monster_try_move(ecs, game, map, monster_id, move_pos, entities, entity_count);
}

ecs_ret_t monster_sys(ecs_t* ecs,
                      ecs_id_t* entities,
                      int entity_count,
                      ecs_dt_t dt,
                      void* udata)
{
    (void)dt;

    game_t* game = udata;

    if (0 == entity_count)
        return LEVEL_OVER;

    int level = game->level;
    tilemap_t* map = game->maps[level];

    ecs_id_t player_id = game->player_id;

    for (int i = 0; i < entity_count; i++)
    {
        ecs_id_t monster_id = entities[i];

        monster_t* monster = ecs_get(ecs, monster_id, MONSTER_COMP);

        switch (monster->state)
        {
            case MONSTER_WANDER:
                monster_do_wander(ecs, game, map, player_id, monster_id, entities, entity_count);
                break;

            case MONSTER_PURSUE:
                monster_do_pursue(ecs, game, map, player_id, monster_id, entities, entity_count);
                break;

            case MONSTER_ATTACK:
                monster_do_attack(game, player_id, monster_id);
                break;

            case MONSTER_FLEE:
                monster_do_flee(ecs, game, map, player_id, monster_id, entities, entity_count);
                break;

            default:
                assert(false);
                break;
        }

        if (game->player_dead)
            return GAME_OVER;
    }

    return 0;
}

ecs_ret_t chest_sys(ecs_t* ecs,
               ecs_id_t* entities,
               int entity_count,
               ecs_dt_t dt,
               void* udata)
{

    (void)dt;

    game_t* game = udata;

    pos_t* player_pos = ecs_get(ecs, game->player_id, POS_COMP);

    for (int i = 0; i < entity_count; i++)
    {
        pos_t* chest_pos = ecs_get(ecs, entities[i], POS_COMP);

        if (distance(player_pos, chest_pos) == 0)
        {
            chest_comp_t* chest = ecs_get(ecs, entities[i], CHEST_COMP);
            stats_t* stats = ecs_get(ecs, game->player_id, STATS_COMP);
            stats->health  += chest->health;
            stats->attack  += chest->attack;
            stats->defense += chest->defense;
            ecs_queue_destroy(ecs, entities[i]);
        }
        else if (distance(player_pos, chest_pos) <= 2)
        {
            drawable_t* drawable = ecs_get(ecs, entities[i], DRAWABLE_COMP);
            drawable->visible = true;
        }
    }


    return 0;
}

ecs_ret_t draw_sys(ecs_t* ecs,
              ecs_id_t* entities,
              int entity_count,
              ecs_dt_t dt,
              void* udata)
{
    (void)dt;
    (void)udata;

    for (int i = 0; i < entity_count; i++)
    {
        ecs_id_t id = entities[i];

        pos_t* pos = ecs_get(ecs, id, POS_COMP);
        drawable_t* drawable = ecs_get(ecs, id, DRAWABLE_COMP);

        if (drawable->visible)
        {
            mvprintw(pos->y, pos->x, "%c", drawable->symbol);
            refresh();
        }
    }

    return 0;
}

void register_systems(game_t* game)
{
    // Player turn system
    PLAYER_SYS = ecs_register_system(game->ecs, player_sys, NULL, NULL, game);

    ecs_require_component(game->ecs, PLAYER_SYS, POS_COMP);
    ecs_require_component(game->ecs, PLAYER_SYS, DRAWABLE_COMP);
    ecs_require_component(game->ecs, PLAYER_SYS, STATS_COMP);
    ecs_require_component(game->ecs, PLAYER_SYS, MONSTER_COMP);

    // Monster's turns
    MONSTER_SYS = ecs_register_system(game->ecs, monster_sys, NULL, NULL, game);

    ecs_require_component(game->ecs, MONSTER_SYS, POS_COMP);
    ecs_require_component(game->ecs, MONSTER_SYS, DRAWABLE_COMP);
    ecs_require_component(game->ecs, MONSTER_SYS, STATS_COMP);
    ecs_require_component(game->ecs, MONSTER_SYS, MONSTER_COMP);

    // Chest system
    CHEST_SYS = ecs_register_system(game->ecs, chest_sys, NULL, NULL, game);

    ecs_require_component(game->ecs, CHEST_SYS, CHEST_COMP);
    ecs_require_component(game->ecs, CHEST_SYS, POS_COMP);
    ecs_require_component(game->ecs, CHEST_SYS, DRAWABLE_COMP);

    // Drawable system
    DRAWABLE_SYS = ecs_register_system(game->ecs, draw_sys, NULL, NULL, game);

    ecs_require_component(game->ecs, DRAWABLE_SYS, POS_COMP);
    ecs_require_component(game->ecs, DRAWABLE_SYS, DRAWABLE_COMP);

}

