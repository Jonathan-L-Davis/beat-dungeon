#include "main.h"

#include "chess/space-chess.h"
#include "graphics/STL/STL.h"

bool quit;
app_timer app_clock;

int main(int argc,char** argv){
    
    init("gui_base");

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
