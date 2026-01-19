#include "main.h"
#include "graphics/graphics.h"
#include "init.h"

#define keyRegistrarDetails
#include "engine/io/keyboard.h"
extern keyRegistrar keyboard;

#include "game/beat-dungeon.h"

extern key up,down,left,right;
init_data_t init_data;
extern board b;

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
    
    if(atlas.FindChildElem("floor")){
        init_data.atlas.floor.x = std::stoi(atlas.GetChildAttrib("x"));
        init_data.atlas.floor.y = std::stoi(atlas.GetChildAttrib("y"));
        init_data.atlas.floor.h = std::stoi(atlas.GetChildAttrib("h"));
        init_data.atlas.floor.w = std::stoi(atlas.GetChildAttrib("w"));
    }else return false;
    
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
        load_atlas(config.GetChildAttrib("file"));
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
    /*// should really not work, but I want it to sooo bad. It's supposed to be an automatically tracked key thing, but it's busted due to initialization order fiasco.
    keyboard.registerKey(&up);
    keyboard.registerKey(&left);
    keyboard.registerKey(&down);
    keyboard.registerKey(&right);//*/
    
    if(!init_data.config_bools.graphics_initialized){
        std::cout << "Failed to complete initialization of graphics module.\n";
        std::exit(-1);
    }
    /*
    b.data.resize(7);
    for(int i = 0; i < b.data.size(); i++)
        b.data[i].resize(7);
    
    for(int i = 0; i < b.data.size(); i++){
        for(int j = 0; j < b.data[i].size(); j++){
            
            
            b.data[i][j].type=floor_t::floor;
            
            if(i&1&&j&1)b.data[i][j].type=floor_t::wall;
            
            
            
        }
    }
    
    b.save_level("lvl1.lvl");//*/std::cout << "loading the level now.\n";
    b.load_level("lvl3.lvl");
}

void terminate(){
    graphics::terminate();
    save_config("config.xml");
}
