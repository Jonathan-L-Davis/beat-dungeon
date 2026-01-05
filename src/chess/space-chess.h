#ifndef SPACE_CHESS_H
#define SPACE_CHESS_H

#include <string>

enum class piece : uint8_t {
    none               = 0,
    pawn               = 1,
    knight             = 2,
    bishop             = 3,
    rook               = 4,
    queen              = 5,
    king               = 6,
    space              = 7
};

enum class color: uint8_t {
    black       = 0x00,// 0x000000
    maroon      = 0x10,// 0x800000
    red         = 0x20,// 0xFF0000
    pink        = 0x30,// 0xFF1493 (hot pink is FF69B4) lol ;) I think I prefer deep pink tho
    orange      = 0x40,// 0xFFA500, FF4500 as an alternative
    yellow      = 0x50,// 0xFFFF00
    lime        = 0x60,// 0x32CD32/00FF00
    green       = 0x70,// 0x008000/006400
    aquamarine  = 0x80,// 0x7FFFD4
    teal        = 0x90,// 0x008080
    blue        = 0xA0,// 0x0000FF
    navy        = 0xB0,// 0x000080
    purple      = 0xC0,// 0x800080
    grey        = 0xD0,// 0x808080
    brown       = 0xE0,// 0x964B00/703F00
    white       = 0xF0 // 0xFFFFFF
};

struct move{
    uint8_t rank_1=0,file_1=0;
    uint8_t rank_2=0,file_2=0;
};

struct board{
    uint8_t files,ranks;
    uint8_t* data;
    
    uint8_t& operator [](int i,int j){
        return (data+(i*files))[j];
    }
    
    bool load(std::string file);
    bool save(std::string file);
    
};

bool is_valid__move(board,move);

#endif//SPACE_CHESS_H
