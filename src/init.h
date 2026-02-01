#ifndef INIT_H
#define INIT_H

#include "util/util.h"
#include <cstdint>
#include <string>
#include <vector>

struct init_data_t{
    
    struct config_bools_t{
        bool general_loaded = false;
        bool config_loaded = false;
        bool graphics_loaded = false;
        bool graphics_initialized = false;
    };
    
    config_bools_t config_bools;
    
    struct rect{uint32_t x,y,h,w;};
    
    struct atlas_t{
        uint32_t h,w;
        rect player,wall_top,wall,drummer,sax,fire,fire_out,floor[4],plate,door_open,door_closed,exit,notes,demon,fireball;
        
        rect plate_tick[4];
        
        rect wall_1_tl_000,wall_1_tl_001,wall_1_tl_010,wall_1_tl_011,wall_1_tl_100,wall_1_tl_101,wall_1_tl_110,wall_1_tl_111;
        rect wall_1_tr_000,wall_1_tr_001,wall_1_tr_010,wall_1_tr_011,wall_1_tr_100,wall_1_tr_101,wall_1_tr_110,wall_1_tr_111;
        rect wall_1_br_000,wall_1_br_001,wall_1_br_010,wall_1_br_011,wall_1_br_100,wall_1_br_101,wall_1_br_110,wall_1_br_111;
        rect wall_1_bl_000,wall_1_bl_001,wall_1_bl_010,wall_1_bl_011,wall_1_bl_100,wall_1_bl_101,wall_1_bl_110,wall_1_bl_111;
        
        rect heiroglyphics[32];
    };
    
    atlas_t atlas;
    
    std::string name;
    std::string GPU;
    std::vector<std::string> GPUS_available;
};

extern init_data_t init_data;

#endif//INIT_H
