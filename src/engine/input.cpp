#include "main.h"

#define keyRegistrarDetails
#include "engine/io/keyboard.h"

#define mouseRegistrarDetails
#include "engine/io/mouse.h"


#include "graphics/imgui/imgui.h"
#include "graphics/imgui/imgui_impl_sdl3.h"

#include <SDL3/SDL.h>

extern keyRegistrar keyboard;
extern SDL_Window* window;

void input(){
    
    SDL_Event event;
    //while (SDL_PollEvent(&event))
    //{
        //ImGui_ImplSDL3_ProcessEvent(&event);
    //}
    //*
    while( SDL_PollEvent( &event ) != 0 )
    {
        //User requests quit
        if (event.type == SDL_EVENT_QUIT)
            quit = true;
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
            quit = true;
        ImGui_ImplSDL3_ProcessEvent(&event);
        
        SDL_Keycode keyEvent = event.key.key;
        if( event.type == SDL_EVENT_KEY_DOWN )
        {
            ///this is a hard coded emergency out key.
            ///Escape always lets you exit the program immediately.
            if( keyEvent == SDLK_ESCAPE){
                    quit = true;
            }
            
            keyboard.pressKey(keyEvent);
        }
        else if( event.type == SDL_EVENT_KEY_UP )
        {
            keyboard.unpressKey(keyEvent);
        }
        if( event.type == SDL_EVENT_MOUSE_BUTTON_DOWN )
        {
            //allMice.pressButton(event.button.button);
        }
        else if( event.type == SDL_EVENT_MOUSE_BUTTON_UP )
        {
            //allMice.unpressButton(event.button.button);
        }
        
        if(event.type == SDL_EVENT_MOUSE_MOTION)
        {
            //allMice.moveMouse(event.motion);
        }
    }//*/
    
}
