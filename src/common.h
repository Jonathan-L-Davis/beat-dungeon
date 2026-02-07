#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <cstdint>
#include <string>

enum struct gamestate_t:uint8_t{
    load = 0,
    main_menu = 1,
    play = 2,
    pause_menu = 3,
    level_select = 4,
    level_editor = 5,
};

#endif//COMMON_H
