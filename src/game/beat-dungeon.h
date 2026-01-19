#pragma once
#include "graphics/graphics.h"
// floor types
enum struct floor_t: uint8_t {
    pit   = 0,
    floor = 1,
    wall = 2,
    plate = 3,// has a count down measured in measures
    door_open = 4,
    door_closed = 5,
    firepit_on = 6,
    firepit_off = 7,
    exit = 8,
};

std::string to_str(floor_t strMe);

struct plate_t{
    uint8_t x,y;// points to door on map.
    uint8_t ticks_alive;
    uint8_t max_ticks;// should only last for up to 3 measures
};

struct cell_t{
    floor_t type;
    void* cell_data;
};

enum : uint64_t {
    player,
    drummer,// has a count down measured in measures
    sax,
    snake,// head or not tracked in entity_data
    notes,// do damage, come from drummer & sax
};

struct entity_t{
    uint64_t type;
    uint32_t x,y;
    void* entity_data;
};

struct player_t{
    uint32_t x,y;
    bool moving_up,moving_left,moving_down,moving_right;
    bool fling_uL,fling_dL,fling_uR,fling_dR;
    bool fling_lU,fling_rU,fling_lD,fling_rD;
};

struct drummer_t{// attacks in a 1 or 2 block "square radius"
    ;
};

struct sax_t{// attacks in a straight line
    ;
};

entity_t create_player();

struct board{
    player_t player;
    //std::vector<entity_t> entities;
    std::vector<std::vector<cell_t>> data;
    
    bool load_level(std::string file_name);
    bool save_level(std::string file_name);
    bool is_flingable(uint32_t x,uint32_t y);
    bool is_walkable(uint32_t x,uint32_t y);
    
    void resize(int x, int y);
    
    void step_player(uint8_t movement);
    void step_plates(int beat);
    void step(int beat,uint8_t movement);
    
    
    
};

// should maybe return a bool?
// is called 4 times per tick.

std::vector<graphics::vulkan::Vertex> draw_board(board b, float aspect_ratio);

