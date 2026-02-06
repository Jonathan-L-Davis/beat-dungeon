#include "main.h"
#include "graphics/graphics.h"
#include "init.h"

#define keyRegistrarDetails
#include "engine/io/keyboard.h"
extern keyRegistrar keyboard;
#define mouseRegistrarDetails
#include "io/mouse.h"
extern mouseRegistrar allMice;

#include "game/beat-dungeon.h"

#include "audio/audio.h"
#include <cmath>

#include <thread>

#include <iostream>

extern key up,down,left,right;
extern mouse_t mouse;

init_data_t init_data;

extern board b;

std::thread audio_thread;

bool load_atlas(std::string path){
    CMarkup atlas;
    if(!atlas.Load(path))
        return false;
    
    atlas.FindElem();
    
    init_data.atlas.h = std::stoi(atlas.GetAttrib("h"));
    init_data.atlas.w = std::stoi(atlas.GetAttrib("w"));
    
    if(atlas.FindChildElem("player")){
        init_data.atlas.player.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.player.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.player.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.player.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_top")){
        init_data.atlas.wall_top.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_top.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_top.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_top.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall")){
        init_data.atlas.wall.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("drummer")){
        init_data.atlas.drummer.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.drummer.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.drummer.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.drummer.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("sax")){
        init_data.atlas.sax.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.sax.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.sax.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.sax.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("fire")){
        init_data.atlas.fire.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.fire.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.fire.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.fire.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("fire_out")){
        init_data.atlas.fire_out.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.fire_out.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.fire_out.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.fire_out.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("bird_bath")){
        init_data.atlas.bird_bath.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.bird_bath.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.bird_bath.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.bird_bath.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    for(int i = 0; i < 4; i++){
        if(atlas.FindChildElem("floor")){
            init_data.atlas.floor[i].x = std::stoi(atlas.GetChildAttrib("x"));
            init_data.atlas.floor[i].y = std::stoi(atlas.GetChildAttrib("y"));
            init_data.atlas.floor[i].h = std::stoi(atlas.GetChildAttrib("h"));
            init_data.atlas.floor[i].w = std::stoi(atlas.GetChildAttrib("w"));
        }else return false;
    }
    
    if(atlas.FindChildElem("plate")){
        init_data.atlas.plate.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.plate.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.plate.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.plate.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("door_closed")){
        init_data.atlas.door_closed.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.door_closed.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.door_closed.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.door_closed.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("door_open")){
        init_data.atlas.door_open.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.door_open.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.door_open.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.door_open.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("exit")){
        init_data.atlas.exit.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.exit.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.exit.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.exit.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("notes")){
        init_data.atlas.notes.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.notes.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.notes.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.notes.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("demon")){
        init_data.atlas.demon.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.demon.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.demon.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.demon.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("fireball")){
        init_data.atlas.fireball.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.fireball.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.fireball.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.fireball.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    for(int i = 0; i < 4; i++){
        if(atlas.FindChildElem("plate_tick")){
            init_data.atlas.plate_tick[i].x = std::stoi(atlas.GetChildAttrib("x"));
            init_data.atlas.plate_tick[i].y = std::stoi(atlas.GetChildAttrib("y"));
            init_data.atlas.plate_tick[i].h = std::stoi(atlas.GetChildAttrib("h"));
            init_data.atlas.plate_tick[i].w = std::stoi(atlas.GetChildAttrib("w"));
        }else return false;
    }
    
    if(atlas.FindChildElem("wall_1_tl_000")){
        init_data.atlas.wall_1_tl_000.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tl_000.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tl_000.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tl_000.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_tl_001")){
        init_data.atlas.wall_1_tl_001.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tl_001.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tl_001.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tl_001.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_tl_010")){
        init_data.atlas.wall_1_tl_010.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tl_010.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tl_010.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tl_010.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_tl_011")){
        init_data.atlas.wall_1_tl_011.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tl_011.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tl_011.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tl_011.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_tl_100")){
        init_data.atlas.wall_1_tl_100.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tl_100.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tl_100.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tl_100.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_tl_101")){
        init_data.atlas.wall_1_tl_101.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tl_101.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tl_101.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tl_101.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_tl_110")){
        init_data.atlas.wall_1_tl_110.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tl_110.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tl_110.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tl_110.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_tl_111")){
        init_data.atlas.wall_1_tl_111.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tl_111.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tl_111.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tl_111.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_tr_000")){
        init_data.atlas.wall_1_tr_000.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tr_000.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tr_000.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tr_000.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_tr_001")){
        init_data.atlas.wall_1_tr_001.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tr_001.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tr_001.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tr_001.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_tr_010")){
        init_data.atlas.wall_1_tr_010.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tr_010.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tr_010.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tr_010.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_tr_011")){
        init_data.atlas.wall_1_tr_011.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tr_011.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tr_011.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tr_011.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_tr_100")){
        init_data.atlas.wall_1_tr_100.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tr_100.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tr_100.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tr_100.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_tr_101")){
        init_data.atlas.wall_1_tr_101.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tr_101.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tr_101.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tr_101.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_tr_110")){
        init_data.atlas.wall_1_tr_110.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tr_110.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tr_110.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tr_110.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_tr_111")){
        init_data.atlas.wall_1_tr_111.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_tr_111.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_tr_111.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_tr_111.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_br_000")){
        init_data.atlas.wall_1_br_000.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_br_000.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_br_000.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_br_000.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_br_001")){
        init_data.atlas.wall_1_br_001.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_br_001.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_br_001.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_br_001.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_br_010")){
        init_data.atlas.wall_1_br_010.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_br_010.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_br_010.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_br_010.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_br_011")){
        init_data.atlas.wall_1_br_011.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_br_011.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_br_011.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_br_011.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_br_100")){
        init_data.atlas.wall_1_br_100.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_br_100.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_br_100.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_br_100.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_br_101")){
        init_data.atlas.wall_1_br_101.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_br_101.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_br_101.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_br_101.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_br_110")){
        init_data.atlas.wall_1_br_110.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_br_110.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_br_110.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_br_110.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_br_111")){
        init_data.atlas.wall_1_br_111.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_br_111.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_br_111.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_br_111.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_bl_000")){
        init_data.atlas.wall_1_bl_000.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_bl_000.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_bl_000.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_bl_000.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_bl_001")){
        init_data.atlas.wall_1_bl_001.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_bl_001.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_bl_001.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_bl_001.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_bl_010")){
        init_data.atlas.wall_1_bl_010.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_bl_010.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_bl_010.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_bl_010.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_bl_011")){
        init_data.atlas.wall_1_bl_011.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_bl_011.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_bl_011.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_bl_011.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_bl_100")){
        init_data.atlas.wall_1_bl_100.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_bl_100.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_bl_100.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_bl_100.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_bl_101")){
        init_data.atlas.wall_1_bl_101.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_bl_101.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_bl_101.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_bl_101.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_bl_110")){
        init_data.atlas.wall_1_bl_110.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_bl_110.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_bl_110.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_bl_110.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    if(atlas.FindChildElem("wall_1_bl_111")){
        init_data.atlas.wall_1_bl_111.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.wall_1_bl_111.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.wall_1_bl_111.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.wall_1_bl_111.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
    for(int i = 0; i < 32; i++){
        if(atlas.FindChildElem("heiroglyphics")){
            init_data.atlas.heiroglyphics[i].x = std::stoi(atlas.GetChildAttrib("x"));
            init_data.atlas.heiroglyphics[i].y = std::stoi(atlas.GetChildAttrib("y"));
            init_data.atlas.heiroglyphics[i].h = std::stoi(atlas.GetChildAttrib("h"));
            init_data.atlas.heiroglyphics[i].w = std::stoi(atlas.GetChildAttrib("w"));
        }else return false;
    }
    
    return true;
}

bool load_config(std::string path){
    CMarkup config;
    if(!config.Load(path))
        return false;
    
    if(config.FindChildElem("general")){
        config.IntoElem();
        
        if(config.FindChildElem("display-name"))
            init_data.name = config.GetChildData();
        init_data.config_bools.general_loaded = true;
        
        config.OutOfElem();
    }
    
    if(config.FindChildElem("atlas")){
        if( !load_atlas(config.GetChildAttrib("file")) ) std::cout << "File output.\n";
    }
    
    if(config.FindChildElem("graphics")){
        config.IntoElem();
        
        if(config.FindChildElem("GPU"))
            init_data.GPU = config.GetChildData();
        
        init_data.config_bools.graphics_loaded = true;
        
        config.OutOfElem();
    }
    
    bool loaded_keybinds = false;
    if(config.FindChildElem("keybinds")){
        config.IntoElem();
        
        /*
        if(config.FindChildElem("GPU"))
            init_data.GPU = config.GetChildData();//*/
        
        loaded_keybinds = true;
        
        config.OutOfElem();
    }
    
    return true;
}

bool save_config(std::string path){
    return false;
}

void init(std::string title){
    
    init_data.config_bools.config_loaded = load_config("config.xml");
    init_data.config_bools.graphics_initialized = graphics::initialize();
    
    up.setKeys({SDLK_W,SDLK_UP});
    left.setKeys({SDLK_A,SDLK_LEFT});
    down.setKeys({SDLK_S,SDLK_DOWN});
    right.setKeys({SDLK_D,SDLK_RIGHT});
    //*// should really not work, but I want it to sooo bad. It's supposed to be an automatically tracked key thing, but it's busted due to initialization order fiasco.
    keyboard.registerKey(&up);
    keyboard.registerKey(&left);
    keyboard.registerKey(&down);
    keyboard.registerKey(&right);//*/
    
    allMice.registerMouse(&mouse);
    
    if(!init_data.config_bools.graphics_initialized){
        std::cout << "Failed to complete initialization of graphics module.\n";
        std::exit(-1);
    }
    
    init_sound();
    audio_thread = std::thread(audio_func);
    
    if(!b.load_level("levels/lvl1.lvl")){
        b.data.resize(17);
        for(int i = 0; i < b.data.size(); i++)
            b.data[i].resize(15);
        
        for(int i = 0; i < b.data.size(); i++){
            for(int j = 0; j < b.data[i].size(); j++){
                
                
                b.data[i][j].type=tile_t::floor;
                
                //if(i&1&&j&1)b.data[i][j].type=floor_t::wall;
                //if(i&1&&j&1)b.data[i][j].cell_data = new wall_t{};
                
                
            }
        }
    }
    
    srand(0);// literally doesn't matter. It's only used for randomness in the generation of floor tiles in the editor.
    
}

void terminate(){
    audio_thread.join();
    graphics::terminate();
    save_config("config.xml");
}
