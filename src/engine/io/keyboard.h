#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <vector>
#include <SDL3/SDL_keycode.h>

class key{
private:
    std::vector<SDL_Keycode> key_addresses;
    std::vector<bool>        booleanKeyPresses;
    bool pressedDown;
    bool isKeyActive;
    uint64_t flags;

public:
    key();
    key(std::vector<SDL_Keycode>);
    key(std::vector<SDL_Keycode>, uint64_t);

    ~key();

    void pressKey(SDL_Keycode);
    void unpressKey(SDL_Keycode);
    
    void deactivate();
    
    bool isPressed();
    bool isActive();

    void setKeys(std::vector<SDL_Keycode>);
    void eraseKeys();
    void setFlags(uint64_t);//no erase because that would just set back to default flag, maybe should have it anyway
};

/**                                           **\
*       Press/Release any Keys                  *
*       Press any Key                           *
*       Hold any Key             ---Default     *
*       Hold Every Key                          *
*                                               *
\**                                           **/

enum keyFlags:uint64_t{
    HOLD_ANY_KEY,
    LAZY_KEYS,//default
    TOGGLE,
};

//uncomment this for better syntax highlighting, recomment when done editing
//#define keyRegistrarDetails
#ifdef keyRegistrarDetails
class keyRegistrar{
private:
    std::vector<key*> trackedKeys;
public:
    void registerKey(key*);
    void unregisterKey(key*);

    void pressKey(SDL_Keycode);
    void unpressKey(SDL_Keycode);
};
#endif//keyRegistrarDetails

#endif//KEYBOARD_H
