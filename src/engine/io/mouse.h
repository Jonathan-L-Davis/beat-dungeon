#ifndef MOUSE_H
#define MOUSE_H

#include <vector>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_events.h>

struct mouse_t{
public:
    bool leftDown;
    bool middleDown;
    bool rightDown;

    float mouseX, mouseY;

    bool isMotionRelative = false;

    mouse_t();
    ~mouse_t();

    void pressButton( decltype(SDL_BUTTON_LEFT) );
    void unpressButton( decltype(SDL_BUTTON_LEFT) );
    void moveMouse(SDL_MouseMotionEvent);
};

//uncomment this for better syntax highlighting, recomment when done editing
#define mouseRegistrarDetails
#ifdef mouseRegistrarDetails
class mouseRegistrar{
private:
    std::vector<mouse_t*> mice;
public:
    void registerMouse(mouse_t*);
    void unregisterMouse(mouse_t*);

    void pressButton( decltype(SDL_BUTTON_LEFT) );
    void unpressButton( decltype(SDL_BUTTON_LEFT) );
    void moveMouse(SDL_MouseMotionEvent);
};
#endif//mouseRegistrarDetails

#endif//MOUSE_H
