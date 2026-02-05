/**
* This define gives us access to the key tracker object, which is disabled by default.
* It is basically a rudimentary garbage collector for the key objects,
* and also makes adding new key inputs a little easier.
*
* I don't want it to be accessible to the front end files,
* but it also needs to be tied to the key input object,
* and I don't want to add another file just for this.
* I want as little attention drawn to this as possible,
* unless you are actively working on the low level key input.
*
* kinda shitty design, will redo eventually
*
* by having the key objects "register" before the main function,
* we use undefined behavior, depending on the compiler to not erase memory before main startup
* not all compilers do this, g++ on linux mint worked nicely, but mingw g++ overwrote with 0, erasing keys.
* must explicitly call all
**/
#define keyRegistrarDetails
#include "keyboard.h"

#include <iostream>
#include <algorithm>

keyRegistrar keyboard;

////////////////////////////////////////////////////////////////////////////////
//                                 key Object                                 //
////////////////////////////////////////////////////////////////////////////////

key::key(){
    key_addresses = {};
    pressedDown = false;
    isKeyActive = false;
    flags = HOLD_ANY_KEY;

    //keyboard.registerKey(this);// relies on undefined behavior. Might get away with it since it's in the same translation unit, but still bad practice.
}

key::key(std::vector<SDL_Keycode> Key_Addresses){
    key_addresses = Key_Addresses;
    booleanKeyPresses.resize(key_addresses.size());
    pressedDown = false;
    isKeyActive = false;
    flags = HOLD_ANY_KEY;

    keyboard.registerKey(this);
}

key::key(std::vector<SDL_Keycode> Key_Addresses, uint64_t FLAGS){
    key_addresses = Key_Addresses;
    booleanKeyPresses.resize(key_addresses.size());
    pressedDown = false;
    isKeyActive = false;
    flags = FLAGS;

    keyboard.registerKey(this);
}

key::~key(){
    keyboard.unregisterKey(this);
}






void key::pressKey(SDL_Keycode keyPressed){
    
    for(unsigned int i = 0; i < key_addresses.size(); i++){
        if(key_addresses[i] == keyPressed){
            booleanKeyPresses[i] = true;
        }
    }

    bool isAnyKeyPressed = false;
    for(unsigned int i = 0; i < booleanKeyPresses.size(); i++){
        isAnyKeyPressed |= booleanKeyPresses[i];
    }
    
    pressedDown = isAnyKeyPressed;
    
    switch(flags){
        default:
        case HOLD_ANY_KEY:
        {
            isKeyActive = isAnyKeyPressed;
        }
        break;
        case TOGGLE:
        {            
            pressedDown = isAnyKeyPressed;
            
            if(!pressedDown && isAnyKeyPressed)
                isKeyActive = !isKeyActive;
        }
        break;
    }
}

void key::unpressKey(SDL_Keycode keyReleased){
    switch(flags){

        case HOLD_ANY_KEY:
        {
            for(unsigned int i = 0; i < key_addresses.size(); i++){
                if(key_addresses[i] == keyReleased){
                    booleanKeyPresses[i] = false;
                }
            }
            bool isAnyKeyPressed = false;
            for(unsigned int i = 0; i < booleanKeyPresses.size(); i++){
                isAnyKeyPressed |= booleanKeyPresses[i];
            }
            isKeyActive = isAnyKeyPressed;
        }
        break;
        case TOGGLE:
        {
            for(unsigned int i = 0; i < key_addresses.size(); i++){
                if(key_addresses[i] == keyReleased){
                    booleanKeyPresses[i] = false;
                }
            }
            bool isAnyKeyPressed = false;
            for(unsigned int i = 0; i < booleanKeyPresses.size(); i++){
                isAnyKeyPressed |= booleanKeyPresses[i];
            }//these two for loops clear the button presses
            pressedDown = isAnyKeyPressed;
        }
        break;
        default:
            for(unsigned int i = 0; i < key_addresses.size(); i++){
                if(key_addresses[i] == keyReleased){
                    pressedDown = false;
                }
            }
    }
}

bool key::isPressed(){
    return pressedDown;
}

bool key::isActive(){
    return isKeyActive;
}


void key::setKeys(std::vector<SDL_Keycode> keys){
    key_addresses = keys;
    booleanKeyPresses.resize(keys.size());
}

void key::eraseKeys(){
    key_addresses = {};
    booleanKeyPresses = {};
}

void key::setFlags(uint64_t Flags){
    flags = Flags;
}

////////////////////////////////////////////////////////////////////////////////
//                             key Tracker object                             //
////////////////////////////////////////////////////////////////////////////////

void keyRegistrar::registerKey(key* registerMe){
    trackedKeys.push_back(registerMe);
}

void keyRegistrar::unregisterKey(key* unregisterMe){
    trackedKeys.erase(std::remove_if(   trackedKeys.begin(),
                                        trackedKeys.end(),
                                        [unregisterMe](key* checkMe){ return checkMe == unregisterMe;} ),
                                        trackedKeys.end());
}

void keyRegistrar::pressKey(SDL_Keycode pressMe){
    for(unsigned int i = 0; i < trackedKeys.size(); i++){
        trackedKeys[i]->pressKey(pressMe);
    }
}

void keyRegistrar::unpressKey(SDL_Keycode pressMe){
    for(unsigned int i = 0; i < trackedKeys.size(); i++){
        trackedKeys[i]->unpressKey(pressMe);
    }
}
