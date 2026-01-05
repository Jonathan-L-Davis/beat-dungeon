#include "main.h"
#include "graphics/graphics.h"
#include "init.h"

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
    
    
    if(!init_data.config_bools.graphics_initialized){
        std::cout << "Failed to complete initialization of graphics module.\n";
        std::exit(-1);
    }
}

void terminate(){
    graphics::terminate();
    save_config("config.xml");
}
