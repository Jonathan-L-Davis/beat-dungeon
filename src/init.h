#ifndef INIT_H
#define INIT_H

#include "util/util.h"
#include <string>
#include <vector>

struct{
    
    struct{
        bool general_loaded = false;
        bool config_loaded = false;
        bool graphics_loaded = false;
        bool graphics_initialized = false;
    }config_bools;
    
    std::string name;
    std::string GPU;
    std::vector<std::string> GPUS_available;
}init_data;

#endif//INIT_H
