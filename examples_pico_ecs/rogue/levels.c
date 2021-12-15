#include "game.h"

#include "json.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

int load_int(const char* name, struct json_object_s* obj)
{
    assert(NULL != obj);

    struct json_object_element_s* elem = obj->start;

    while (NULL != elem)
    {
        if (0 == strcmp(name, elem->name->string))
        {
            struct json_number_s* num = json_value_as_number(elem->value);
            return (int)atof(num->number);
        }

        elem = elem->next;
    }

    assert(false);
    return 0;
}

struct json_object_s* load_object(const char* name, struct json_object_s* parent)
{
    assert(NULL != parent);

    struct json_object_element_s *elem = parent->start;

    while (elem)
    {
        if (0 == strcmp(elem->name->string, name))
        {
            return json_value_as_object(elem->value);
        }

        elem  = elem->next;
    }

    assert(false);
    return NULL;
}

struct json_array_s* load_array(const char* name, struct json_object_s* parent)
{
    assert(NULL != parent);

    struct json_object_element_s* elem = parent->start;

    while (elem)
    {
        if (0 == strcmp(elem->name->string, name))
        {
            return json_value_as_array(elem->value);
        }

        elem  = elem->next;
    }

    assert(false);
    return NULL;
}

pos_t load_pos(struct json_object_s* obj)
{
    assert(NULL != obj);

    pos_t pos = { 0 };

    pos.x = load_int("x", obj);
    pos.y = load_int("y", obj);

    return pos;
}

dim_t load_dim(struct json_object_s* obj)
{
    assert(NULL != obj);

    dim_t dim = { 0 };

    dim.w = load_int("w", obj);
    dim.h = load_int("h", obj);

    return dim;
}

room_t load_room(struct json_object_s* obj)
{
    assert(NULL !=  obj);

    room_t room = { 0 };

    room.pos = load_pos(load_object("pos", obj));
    room.dim = load_dim(load_object("dim", obj));

    return room;
}

pos_t* load_corners(struct json_array_s* array, int* corner_count)
{
    int count = array->length;
    pos_t* corners = malloc(count * sizeof(pos_t));

    struct json_array_element_s* elem = array->start;

    for (int i = 0; i < count; i++)
    {
        corners[i] = load_pos(json_value_as_object(elem->value));
        elem = elem->next;
    }

    *corner_count = count;

    return corners;
}

tunnel_t load_tunnel(struct json_array_s* array)
{
    tunnel_t tunnel = { 0 };

    int corner_count = 0;
    tunnel.corners = load_corners(array, &corner_count);
    tunnel.corner_count = corner_count;

    return tunnel;
}

npc_t load_npc(struct json_object_s* obj)
{
    assert(NULL !=  obj);

    npc_t npc = { 0 };

    npc.pos = load_pos(load_object("pos", obj));
    npc.type = load_int("type", obj);

    return npc;
}

chest_t load_chest(struct json_object_s* obj)
{
    assert(NULL !=  obj);

    chest_t chest = { 0 };

    chest.pos = load_pos(load_object("pos", obj));

    return chest;
}

level_t* load_level(const char* json_str)
{
    level_t* level = malloc(sizeof(level_t));

    struct json_value_s*  root = json_parse(json_str, strlen(json_str));
    struct json_object_s* obj  = json_value_as_object(root);

    // Player
    level->player = load_pos(load_object("player", obj));

    // Load rooms
    struct json_array_s* array        = load_array("rooms", obj);
    struct json_array_element_s* elem = array->start;

    level->room_count = array->length;
    level->rooms = malloc(array->length * sizeof(room_t));

    for (size_t i = 0; i < array->length; i++)
    {
        level->rooms[i] = load_room(json_value_as_object(elem->value));
        elem = elem->next;
    }

    // Load tunnels
    array = load_array("tunnels", obj);
    elem  = array->start;

    level->tunnel_count = array->length;
    level->tunnels = malloc(array->length * sizeof(tunnel_t));

    for (size_t i = 0; i < array->length; i++)
    {
        level->tunnels[i] = load_tunnel(json_value_as_array(elem->value));
        elem = elem->next;
    }

    // Load NPCs
    array = load_array("npcs", obj);
    elem  = array->start;

    level->npc_count = array->length;
    level->npcs = malloc(array->length * sizeof(npc_t));

    for (size_t i = 0; i < array->length; i++)
    {
        level->npcs[i] = load_npc(json_value_as_object(elem->value));
        elem = elem->next;
    }

    // Load Chests
    array = load_array("chests", obj);
    elem  = array->start;

    level->chest_count = array->length;
    level->chests = malloc(array->length * sizeof(chest_t));

    for (size_t i = 0; i < array->length; i++)
    {
        level->chests[i] = load_chest(json_value_as_object(elem->value));
        elem = elem->next;
    }

    free(root);

    return level;
}

void free_level(level_t* level) // TODO: destroy_level
{
    free(level->rooms);

    for (int i = 0; i < level->tunnel_count; i++)
    {
        free (level->tunnels[i].corners);
    }

    free(level->tunnels);
    free(level->npcs);
    free(level);
}

