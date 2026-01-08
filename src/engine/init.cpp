#include "main.h"
#include "graphics/graphics.h"
#include "init.h"

#define keyRegistrarDetails
#include "engine/io/keyboard.h"
extern keyRegistrar keyboard;

#include "game/beat-dungeon.h"

extern key up,down,left,right;

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
}

void terminate(){
    graphics::terminate();
    save_config("config.xml");
}
