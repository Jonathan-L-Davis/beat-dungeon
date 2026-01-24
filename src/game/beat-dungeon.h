#pragma once
#include "graphics/graphics.h"
// floor types
enum struct tile_t: uint8_t {
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

std::string to_str(tile_t strMe);

struct plate_t{
    uint8_t character;
    uint8_t ticks_alive;
    uint8_t max_ticks;// should only last for up to 3 measures
};

struct door_t{
    uint8_t character;
};

struct pit_t{
    uint8_t character;
};

struct firepit_t{
    uint8_t variation;
};

struct wall_t{
    uint8_t surroundings;
    uint8_t character;// controls which glyph to render (if any)
    uint8_t variation;
};

struct floor_t{
    uint8_t variation;
};

struct exit_t{
    uint8_t variation;
};

struct cell_t{
    tile_t type;
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
    
    bool dead = false,won = false;
    uint32_t anim_phase = 0;
};

struct drummer_t{// attacks in a 1 or 2 block "square radius"
    ;
};

struct sax_t{// attacks in a straight line
    uint32_t x,y;
    uint8_t movement;
    uint8_t shot_cooldown;
};

struct drum_t{// attacks in a straight line
    uint32_t x,y;
    uint8_t movement;
};

struct notes_t{// attacks in a straight line
    uint32_t x,y;
    uint8_t movement;
};

struct demon_t{// attacks in a straight line
    uint32_t x,y;
    uint8_t movement;
};

struct fireball_t{// attacks in a straight line
    uint32_t x,y;
    uint8_t movement;
};



entity_t create_player();

struct board{
    player_t player;
    //std::vector<entity_t> entities;
    std::string level;
    std::vector<std::vector<cell_t>> data;
    std::vector<sax_t> saxophones;
    std::vector<drum_t> drums;
    std::vector<notes_t> notes;
    std::vector<demon_t> demons;
    std::vector<fireball_t> fireballs;
    
    bool load_level(std::string file_name);
    bool save_level(std::string file_name);
    bool is_flingable(uint32_t x,uint32_t y);
    bool is_walkable(uint32_t x,uint32_t y);
    bool is_visible(uint32_t x,uint32_t y);
    
    void resize(int x, int y);
    
    void step_player(int beat,uint8_t movement);
    void step_saxophone(int beat);
    void step_plates(int beat);
    void step_notes(int beat);
    void step(int beat,uint8_t movement);
    
    void update_wall_borders();
    
};

// should maybe return a bool?
// is called 4 times per tick.

std::vector<graphics::vulkan::Vertex> draw_board(board b, float aspect_ratio);

