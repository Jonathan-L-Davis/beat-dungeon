#pragma once
#include "graphics/graphics.h"
#include <cstdint>
// floor types
enum struct tile_t: uint8_t {
    pit         = 0,
    floor       = 1,
    wall        = 2,
    plate       = 3,
    door_open   = 4,
    door_closed = 5,
    firepit_on  = 6,
    firepit_off = 7,
    exit        = 8,
    bird_bath   = 9,// lol, iykyk
};

std::string to_str(tile_t strMe);
bool tile_kinds_match(tile_t a,tile_t b);

struct plate_t{
    uint8_t character;
    uint8_t ticks_alive;
    uint8_t max_ticks;// should only last for up to 3 measures
    uint8_t variation;
};

struct door_t{
    uint8_t character;
    uint8_t variation;
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

enum entity_t: uint64_t {
    player   = 0,
    sax      = 1,
    notes    = 2,
    drummer  = 3,
    demon    = 4,
    fireball = 5,
    
};

struct player_t{
    uint32_t x,y;
    bool moving_up,moving_left,moving_down,moving_right;
    bool fling_uL,fling_dL,fling_uR,fling_dR;
    bool fling_lU,fling_rU,fling_lD,fling_rD;
    
    bool dead = false,won = false;
    uint32_t anim_phase = 0;
};

struct sax_t{// attacks in a straight line
    uint32_t x,y;
    uint8_t movement;
    uint8_t shot_cooldown;
};

struct drum_t{// attacks in a straight line
    uint32_t x,y;
    uint8_t movement;
    uint8_t shot_cooldown;
};

struct demon_t{// attacks in a straight line
    uint32_t x,y;
    uint8_t movement;
    uint8_t shot_cooldown;
};

struct camel_t{// attacks in a straight line
    uint32_t x,y;
    uint8_t movement;
    uint8_t shot_cooldown;
};

struct notes_t{// attacks in a straight line
    uint32_t x,y;
    uint8_t movement;
};

struct fireball_t{// attacks in a straight line
    uint32_t x,y;
    uint8_t movement;
};

struct spitball_t{// attacks in a straight line
    uint32_t x,y;
    uint8_t movement;
};

struct board{
    player_t player;
    //std::vector<entity_t> entities;
    std::string level;
    float depth;
    std::vector<std::vector<cell_t>> data;
    std::vector<sax_t> saxophones;
    std::vector<drum_t> drums;
    std::vector<demon_t> demons;
    std::vector<camel_t> camels;
    
    std::vector<notes_t> notes;
    std::vector<fireball_t> fireballs;
    std::vector<spitball_t> spitballs;
    
    bool load_level(std::string file_name);
    bool save_level(std::string file_name);
    
    bool load_level(std::vector<uint8_t>& file);
    bool save_level(std::vector<uint8_t>& file);
    
    bool is_flingable(uint32_t x,uint32_t y);
    bool is_walkable(uint32_t x,uint32_t y);
    bool is_visible(uint32_t x,uint32_t y);
    
    void resize(int x, int y);
    
    void step_player(int beat,uint8_t movement);
    void step_saxophone(int beat);
    void step_demons(int beat);
    void step_plates();
    void step_notes(int beat);
    void step_fireball(int beat);
    void step(int beat,uint8_t movement);
    
    void step_audio();
    
    void update_wall_borders();
    
    struct sound_bools_t{
        bool plate_ticked;
        bool plate_visually_ticked;
        bool plate_pressed;
        bool plate_unpressed;
        
        bool door_opened;
        bool door_closed;
        
        bool player_won;
        bool player_died;
    };
    
    sound_bools_t sound_bools;
    
};

// should maybe return a bool?
// is called 4 times per tick.

std::vector<graphics::vulkan::Vertex> draw_board(board b, float aspect_ratio);

