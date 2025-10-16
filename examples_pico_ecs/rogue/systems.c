#include "components.h"
#include "game.h"
#include "hud.h"
#include "tilemaps.h"

#include "pico_ecs.h"

#include <assert.h>
#include <curses.h>
#include <stdbool.h>
#include <stdio.h>

ecs_sys_t PLAYER_SYS;
ecs_sys_t MONSTER_SYS;
ecs_sys_t CHEST_SYS;
ecs_sys_t DRAWABLE_SYS;

bool get_entity_at(ecs_t* ecs,
                   pos_t pos,
                   ecs_entity_t entities[],
                   int entity_count,
                   ecs_entity_t* entity)
{
    for (int i = 0; i < entity_count; i++)
    {
        pos_t* entity_pos = ecs_get(ecs, entities[i], POS_COMP);

        if (pos_equal(&pos, entity_pos))
        {
            if (NULL != entity)
                *entity = entities[i];

            return true;
        }
    }

    return false;
}

bool perform_attack(game_t* game, ecs_entity_t attacker, ecs_entity_t defender)
{
    ecs_t* ecs = game->ecs;

    bool is_player = ecs_has(ecs, attacker, PLAYER_COMP);

    stats_t* attacker_comp = ecs_get(ecs, attacker, STATS_COMP);
    stats_t* defender_comp = ecs_get(ecs, defender, STATS_COMP);

    int damage  = random_int(0, attacker_comp->attack);
    int defense = defender_comp->defense;
    damage = max(damage - defense, 0);
    defender_comp->health -= damage;

    if (defender_comp->health <= 0)
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
                     ecs_entity_t* entities,
                     int entity_count,
                     void* udata)
{
    game_t* game = udata;

    int level = game->level;
    tilemap_t* map = game->maps[level];

    player_t* player = ecs_get(ecs, game->player, PLAYER_COMP);

    pos_t* player_pos = ecs_get(ecs, game->player, POS_COMP);

    pos_t offset = get_move_offset(player->cmd);
    pos_t pos = pos_add(player_pos, &offset);

    ecs_entity_t target;

    if (get_entity_at(game->ecs, pos, entities, entity_count, &target))
    {
        if (ecs_has(ecs, target, MONSTER_COMP))
        {
            if (perform_attack(game, game->player, target))
                ecs_queue_destroy(ecs, target);
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
                      ecs_entity_t monster,
                      pos_t pos,
                      ecs_entity_t entities[],
                      int entity_count)
{
    pos_t* monster_pos = ecs_get(ecs, monster, POS_COMP);

    if (collides_with_tile(map, pos))
        return false;

    if (get_entity_at(ecs, pos, entities, entity_count, NULL))
        return false;

    pos_t* player_pos = ecs_get(ecs, game->player, POS_COMP);

    if (pos_equal(&pos, player_pos))
        return false;

    *monster_pos = pos;

    return true;
}

void monster_do_wander(ecs_t* ecs,
                       game_t* game,
                       tilemap_t* map,
                       ecs_entity_t player,
                       ecs_entity_t monster,
                       ecs_entity_t entities[],
                       int entity_count)
{
    pos_t* player_pos = ecs_get(ecs, player, POS_COMP);

    monster_t* monster_comp = ecs_get(ecs, monster, MONSTER_COMP);
    pos_t* monster_pos = ecs_get(ecs, monster, POS_COMP);

    pos_t  move_pos = random_move(monster_pos);
    monster_try_move(ecs, game, map, monster, move_pos, entities, entity_count);

    if (distance(player_pos, monster_pos) <= 4)
    {
        draw_monster_msg(game, "Monster pursues!\n");
        monster_comp->state = MONSTER_PURSUE;
        return;
    }
}

void monster_do_pursue(ecs_t* ecs,
                       game_t* game,
                       tilemap_t* map,
                       ecs_entity_t player,
                       ecs_entity_t monster,
                       ecs_entity_t entities[],
                       int entity_count)
{
    pos_t* player_pos = ecs_get(ecs, player, POS_COMP);
    monster_t* monster_comp = ecs_get(ecs, monster, MONSTER_COMP);
    pos_t* monster_pos = ecs_get(ecs, monster, POS_COMP);

    if (distance(player_pos, monster_pos) <= 1)
    {
        draw_monster_msg(game, "Monster attacks!\n");
        monster_comp->state = MONSTER_ATTACK;
        return;
    }

    pos_t move_pos = move_toward(map, monster_pos, player_pos);
    monster_try_move(ecs, game, map, monster, move_pos, entities, entity_count);
}

void monster_do_attack(game_t* game, ecs_entity_t player, ecs_entity_t monster)
{
    pos_t* player_pos = ecs_get(game->ecs, player, POS_COMP);
    monster_t* monster_comp = ecs_get(game->ecs, monster, MONSTER_COMP);
    pos_t* monster_pos = ecs_get(game->ecs, monster, POS_COMP);
    stats_t* monster_stats = ecs_get(game->ecs, monster, STATS_COMP);

    if (monster_stats->health < 3)
    {
        draw_monster_msg(game, "Monster flees...\n");
        monster_comp->state = MONSTER_FLEE;
        return;
    }

    if (distance(player_pos, monster_pos) > 1)
    {
        monster_comp->state = MONSTER_PURSUE;
        return;
    }

    if (perform_attack(game, monster, player))
    {
        game->player_dead = true;
    }
}

void monster_do_flee(ecs_t* ecs,
                     game_t* game,
                     tilemap_t* map,
                     ecs_entity_t player,
                     ecs_entity_t monster,
                     ecs_entity_t entities[],
                     int entity_count)
{
    monster_t* monster_comp = ecs_get(ecs, monster, MONSTER_COMP);
    pos_t* monster_pos = ecs_get(ecs, monster, POS_COMP);
    pos_t* player_pos = ecs_get(ecs, player, POS_COMP);

    if (distance(player_pos, monster_pos) > 3)
    {
        monster_comp->state = MONSTER_WANDER;
        return;
    }

    pos_t move_pos = move_away(map, monster_pos, player_pos);
    monster_try_move(ecs, game, map, monster, move_pos, entities, entity_count);
}

ecs_ret_t monster_sys(ecs_t* ecs,
                      ecs_entity_t* entities,
                      int entity_count,
                      void* udata)
{
    game_t* game = udata;

    if (0 == entity_count)
        return LEVEL_OVER;

    int level = game->level;
    tilemap_t* map = game->maps[level];

    ecs_entity_t player = game->player;

    for (int i = 0; i < entity_count; i++)
    {
        ecs_entity_t monster = entities[i];

        monster_t* monster_comp = ecs_get(ecs, monster, MONSTER_COMP);

        switch (monster_comp->state)
        {
            case MONSTER_WANDER:
                monster_do_wander(ecs, game, map, player, monster, entities, entity_count);
                break;

            case MONSTER_PURSUE:
                monster_do_pursue(ecs, game, map, player, monster, entities, entity_count);
                break;

            case MONSTER_ATTACK:
                monster_do_attack(game, player, monster);
                break;

            case MONSTER_FLEE:
                monster_do_flee(ecs, game, map, player, monster, entities, entity_count);
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
               ecs_entity_t* entities,
               int entity_count,
               void* udata)
{
    game_t* game = udata;

    pos_t* player_pos = ecs_get(ecs, game->player, POS_COMP);

    for (int i = 0; i < entity_count; i++)
    {
        pos_t* chest_pos = ecs_get(ecs, entities[i], POS_COMP);

        if (distance(player_pos, chest_pos) == 0)
        {
            chest_comp_t* chest = ecs_get(ecs, entities[i], CHEST_COMP);
            stats_t* stats = ecs_get(ecs, game->player, STATS_COMP);
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
              ecs_entity_t* entities,
              int entity_count,
              void* udata)
{
    (void)udata;

    for (int i = 0; i < entity_count; i++)
    {
        ecs_entity_t id = entities[i];

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

