#include "main.h"

bool quit;
app_timer app_clock;

#include <iostream>

int main(int argc,char** argv){
    
    std::cout << "init started\n";
    init("beat-dungeon");
    std::cout << "init complete\n";
    
    app_clock.start();
    while( !quit ){
        
        app_clock.accumulate();
        
        while( app_clock.need_update() ){
            input();
            update( app_clock.time_step() );
        }
        
        app_clock.sleep();
        
        if( app_clock.need_frame() ){
            draw();
        }
        
    }
    
    terminate();
}
