#pragma once

// floor types
enum : uint64_t {
    floor,
    plate,// has a count down measured in measures
    door,
    flipper, // walkable after collision, opaque after movement through
    fire,// maybe a combat adjacent tile?
};

struct cell_t{
    uint64_t type;
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
    ;
};

struct drummer_t{// attacks in a 1 or 2 block "square radius"
    ;
};

struct sax_t{// attacks in a straight line
    ;
};

entity_t create_player();

struct board{
    entity_t player;
    std::vector<entity_t> entities;
    std::vector<std::vector<cell_t>> data;
    
    bool load_level(std::string file_name);
};

// should maybe return a bool?
// is called 4 times per tick.
void step_board();
