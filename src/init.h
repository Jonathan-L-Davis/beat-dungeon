#ifndef INIT_H
#define INIT_H

#include "util/util.h"
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
        rect player,wall,drummer,sax,fire,fire_out,floor,plate,door_open,door_closed,exit;
    };
    
    atlas_t atlas;
    
    std::string name;
    std::string GPU;
    std::vector<std::string> GPUS_available;
};

extern init_data_t init_data;

#endif//INIT_H
