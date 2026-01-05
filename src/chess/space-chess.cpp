#include "space-chess.h"
#include "util.h"

bool board::load(std::string file){
    std::vector<uint8_t> data;
    if(!load_file(file,data))
        return false;
    
    if(data.size()==0)
        return false;
    
    int ranks = (data[0]&0x0F>>0)+1;
    int files = (data[0]&0xF0>>4)+1;
    
    int board_size = ranks*files;
    
    if(data.size()<board_size+1)
        return false;
    
    freemem();
    
    data = new (std::nothrow) uint8_t[board_size];
    if(data==nullptr)
        return false;
    
    std::memcpy(data,data.data()+1,board_size);
    
    return true;
}

bool board::save(std::string file){
    
    
}

void board::freemem(){
    if(data!=nullptr)
        delete[] data;
    data = nullptr;
}

board::~board(){
    freemem();
}

bool make_move(board b, move makeMe){
    
    if(makeMe.file_1==makeMe.file_2&&makeMe.rank_1==makeMe.rank_2)
        return false;
    
    piece p1 = b.data[makeMe.file_1][makeMe.rank_1]&0x0F;
    piece p2 = b.data[makeMe.file_2][makeMe.rank_2]&0x0F;
    color c1 = b.data[makeMe.file_1][makeMe.rank_1]&0xF0;
    color c2 = b.data[makeMe.file_2][makeMe.rank_2]&0xF0;
    
    // don't move an empty space as a piece
    if(p1!=piece::none||p1!=piece::space)
        return false;
    
    // don't move to an empty space
    if(p2==piece::space)
        return false;
    
    // don't capture an ally piece
    if(c1==c2&&p2!=piece::none)
        return false;
    
    switch(p1){
        case piece::none:return false;
        case piece::space:return false;
        case piece::pawn:{
            if(!is_valid_pawn_move(b,makeMe))
                return false;
        }break;
        case piece::knight:{
            if(!is_valid_knight_move(b,makeMe))
                return false;
        }break;
        case piece::bishop:{
            if(!is_valid_bishop_move(b,makeMe))
                return false;
        }break;
        case piece::rook:{
            if(!is_valid_rook_move(b,makeMe))
                return false;
        }break;
        case piece::queen:{
            if(!is_valid_queen_move(b,makeMe))
                return false;
        }break;
        case piece::king:{
            if(!is_valid_king_move(b,makeMe))
                return false;
        }break;
        default:return false;
    }
    
    //make the move, it's valid.
    b.data[makeMe.file_1][makeMe.rank_1] = c1|p1;
    b.data[makeMe.file_1][makeMe.rank_1] = c1|piece::none;// leaving the color info because why not.
    
    return true;
}

bool is_valid_rook_move(board b, move m){
    int file_d = m.file_2-m.file_1, rank_d = m.rank_2-m.rank_1;
    
    // can't not move.
    if(file_d==0&&rand_d==0)
        return false;
    
    // A rook can only move in 1 direction at a time.
    if(file_d!=0&&rand_d!=0)
        return false;
    
    // check for premature collisions.
    int file_d_sign = std::abs(file_d)/file_d;
    for(int i = 0; i < std::abs(file_d)-1; i++){
        if(b[m.file_1+(file_d_sign*i)][m.rank_1]&0x0F!=piece::none)// don't want to be trampling through pieces if it's not a capture.
            return false;
    }
    
    // check for premature collisions.
    int rank_d_sign = std::abs(rank_d)/rank_d;
    for(int i = 0; i < std::abs(file_d)-1; i++){
        if(b[m.file_1][m.rank_1+(rank_d_sign*i)]&0x0F!=piece::none)// don't want to be trampling through pieces if it's not a capture.
            return false;
    }
    
    return true;
}

bool is_valid_bishop_move(board b, move m){
    int file_d = m.file_2-m.file_1, rank_d = m.rank_2-m.rank_1;
    
    // can't not move.
    if(file_d==0&&rand_d==0)
        return false;
    
    // A bishop must move in equal amounts along rank and file axes.
    if(std::abs(file_d)!=std::abs(rand_d))
        return false;
    
    // check for premature collisions.
    int file_d_sign = std::abs(file_d)/file_d;
    int rank_d_sign = std::abs(rank_d)/rank_d;
    for(int i = 0; i < std::abs(file_d); i++){
        if(b[m.file_1+(file_d_sign*i)][m.rank_1+(rank_d_sign*i)]&0x0F!=piece::none)// don't want to be trampling through pieces if it's not a capture.
            if(b[m.file_1+(file_d_sign*i)][m.rank_1+(rank_d_sign*i)]&0x0F!=piece::space)//also allow bishops to fly over spaces?
                return false;
    }
    
    return true;
}

bool is_valid_king_move(){
    return std::abs(makeMe.file_1-makeMe.file_2)<2&&std::abs(makeMe.rank_1-makeMe.rank_2)<2;
}
