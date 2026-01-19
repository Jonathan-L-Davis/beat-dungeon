#include "main.h"
#include "io/keyboard.h"
#include "io/mouse.h"

#include <iostream>
#include <chrono>

#include "game/beat-dungeon.h"

int BPM = 90;
constexpr int min_BPM =  60;
constexpr int max_BPM = 120;

std::chrono::time_point<std::chrono::system_clock> last_beat;

board b;

key up,down,left,right;
mouse_t mouse;

void update(double timeStep){
    
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    
    std::chrono::duration<float> duration = now-last_beat;
    
    // these could be more dynamic!!! (I could make a trueBPM variable & allow the sound track to speed up or slow down relative to the BPM.
    std::chrono::duration<float> full_beat          = std::chrono::duration<float>(60.f/BPM);
    std::chrono::duration<float> three_quarter_beat = std::chrono::duration<float>(45.f/BPM);
    std::chrono::duration<float> half_beat          = std::chrono::duration<float>(30.f/BPM);
    std::chrono::duration<float> quarter_beat       = std::chrono::duration<float>(15.f/BPM);
    
    static bool t = false, h = false, q = false;
    
    static uint8_t movement = 0;
    
    movement |= up.isActive()*1;
    movement |= left.isActive()*2;
    movement |= down.isActive()*4;
    movement |= right.isActive()*8;
    
    if(duration>full_beat){
        last_beat = now;
        b.step(4,movement);
        
        movement = 0;
        
        t = false;
        h = false;
        q = false;
        
        return;
    }
    if(duration>three_quarter_beat && !t){
        b.step(3,movement);
        movement = 0;
        t = true;
        return;
    }
    if(duration>half_beat && !h){
        b.step(2,movement);
        movement = 0;
        h = true;
        return;
    }
    if(duration>quarter_beat && !q){
        b.step(1,movement);
        movement = 0;
        q = true;
        return;
    }
    
}
