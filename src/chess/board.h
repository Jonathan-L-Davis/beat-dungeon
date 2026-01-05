#ifndef BOARD_H
#define BOARD_H



struct board{
    uint8_t data[8][8];
};

struct move{
    uint8_t rank_1=0,file_1=0;
    uint8_t rank_2=0,file_2=0;
};

bool is_valid_rook_move(board b, move m);

#endif//BOARD_H

