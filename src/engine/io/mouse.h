#ifndef MOUSE_H
#define MOUSE_H

#include <vector>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_events.h>

struct mouse{
public:
    bool leftDown;
    bool middleDown;
    bool rightDown;

    int mouseX, mouseY;
    float mouseSensitivity = 0.5f;

    bool isMotionRelative;

    mouse();
    ~mouse();

    void pressButton( decltype(SDL_BUTTON_LEFT) );
    void unpressButton( decltype(SDL_BUTTON_LEFT) );
    void moveMouse(SDL_MouseMotionEvent);
};

//uncomment this for better syntax highlighting, recomment when done editing
#define mouseRegistrarDetails
#ifdef mouseRegistrarDetails
class mouseRegistrar{
private:
    std::vector<mouse*> mice;
public:
    void registerMouse(mouse*);
    void unregisterMouse(mouse*);

    void pressButton( decltype(SDL_BUTTON_LEFT) );
    void unpressButton( decltype(SDL_BUTTON_LEFT) );
    void moveMouse(SDL_MouseMotionEvent);
};
#endif//mouseRegistrarDetails

#endif//MOUSE_H
