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
    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL3_ProcessEvent(&event);
        if (event.type == SDL_EVENT_QUIT)
            quit = true;
        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window))
            quit = true;
    }
    /*
    SDL_Event event;
    while( SDL_PollEvent( &event ) != 0 )
    {
        //User requests quit
        if( event.type == SDL_EVENT_QUIT ||
           (event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_EVENT_WINDOW_CLOSE_REQUESTED &&
            event.window.windowID == SDL_GetWindowID(GLcontext.window) )
        ){
            //event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)
            quit = true;
        }
        ImGui_ImplSDL3_ProcessEvent(&event);
        
        SDL_Keycode keyEvent = event.key.keysym.sym;
        if( event.type == SDL_KEYDOWN )
        {
            ///this is a hard coded emergency out key.
            ///Escape always lets you exit the program immediately.
            if( keyEvent == SDLK_ESCAPE){
                    quit = true;
            }
            
            keyboard.pressKey(keyEvent);
        }
        else if( event.type == SDL_KEYUP )
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
