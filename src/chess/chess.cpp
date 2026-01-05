#include "chess.h"

namespace chess{
    
    struct move{
        uint8_t rank_1=0,file_1=0;
        uint8_t rank_2=0,file_2=0;
    };
    
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
            case piece::pawn_unmoved:{
                // only support double moves for black and white pawns right now, because that's complicated.
                if(c1!=color::black&&c1!=color::white)
                    return false;
            }
            case piece::pawn_enpassantable:
            case piece::pawn:{
                if(is_valid_pawn_move(b,makeMe))
                    c1 = piece::pawn;
                if(is_en_passant(b,makeMe))
                    ;// delete the enpassant'ed pawn.
                else
                    return false;
            }break;
            case piece::knight:{
            }break;
            case piece::bishop:{
            }break;
            case piece::rook_unmoved:
            case piece::rook:{
                if(valid_rook_move(b,makeMe))
                    p1 = piece::rook;
                else
                    return false;
            }break;
            case piece::queen:{
            }break;
            case piece::king_unmoved:
            case piece::king:{
                if(std::abs(makeMe.file_1-makeMe.file_2)<2&&std::abs(makeMe.rank_1-makeMe.rank_2)<2)
                    p1 = piece::king;// moving 
                else
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
        if(file_d<0){
            
        }
        
        return true;
    }
    
    bool is_valid_bishop_move(board b, move m){
        int file_d = m.file_2-m.file_1, rank_d = m.rank_2-m.rank_1;
        
        // can't not move.
        if(file_d==0&&rand_d==0)
            return false;
        
        // A rook can only move in 1 direction at a time.
        if(std::abs(file_d)!=std::abs(rand_d))
            return false;
        
        // check for premature collisions.
        
        return true;
    }
    
};
