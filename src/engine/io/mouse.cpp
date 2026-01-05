#define mouseRegistrarDetails
#include "mouse.h"
#include <algorithm>

mouseRegistrar allMice;

mouse::mouse(){
    leftDown = false;
    middleDown = false;
    rightDown = false;

    mouseX = 0;
    mouseY = 0;

    isMotionRelative = true;

    allMice.registerMouse(this);
}

mouse::~mouse(){
    allMice.unregisterMouse(this);
}

void mouse::pressButton( decltype(SDL_BUTTON_LEFT) pressMe ){
    if(pressMe == SDL_BUTTON_LEFT){
        leftDown = true;
    }else if(pressMe == SDL_BUTTON_MIDDLE){
        middleDown = true;
    }else if(pressMe == SDL_BUTTON_RIGHT){
        rightDown = true;
    }
}

void mouse::unpressButton( decltype(SDL_BUTTON_LEFT) unpressMe ){
    if(unpressMe == SDL_BUTTON_LEFT){
        leftDown = false;
    }else if(unpressMe == SDL_BUTTON_MIDDLE){
        middleDown = false;
    }else if(unpressMe == SDL_BUTTON_RIGHT){
        rightDown = false;
    }
}

void mouse::moveMouse(SDL_MouseMotionEvent motion){
    if( isMotionRelative ){
        mouseX = motion.xrel;
        mouseY = motion.yrel;
    }else{
        mouseX = motion.x;
        mouseY = motion.y;
    }
}


void mouseRegistrar::registerMouse(mouse* registerMe){
    mice.push_back(registerMe);
}

void mouseRegistrar::unregisterMouse(mouse* unregisterMe){

    mice.erase(std::remove_if(   mice.begin(),
                                        mice.end(),
                                        [unregisterMe](mouse* checkMe){ return checkMe == unregisterMe;} ),
                                        mice.end());
}

void mouseRegistrar::pressButton( decltype(SDL_BUTTON_LEFT) pressMe ){
    for(unsigned int i = 0; i < mice.size(); i++){
        mice[i]->pressButton(pressMe);
    }
}

void mouseRegistrar::unpressButton( decltype(SDL_BUTTON_LEFT) unpressMe ){
    for(unsigned int i = 0; i < mice.size(); i++){
        mice[i]->unpressButton(unpressMe);
    }
}

void mouseRegistrar::moveMouse( SDL_MouseMotionEvent motion ){
    for(unsigned int i = 0; i < mice.size(); i++){
        mice[i]->moveMouse(motion);
    }
}
