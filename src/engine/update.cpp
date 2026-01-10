#include "main.h"
#include "io/keyboard.h"

#include <iostream>
#include <chrono>

#include "game/beat-dungeon.h"

int BPM = 90;
constexpr int min_BPM =  60;
constexpr int max_BPM = 120;

std::chrono::time_point<std::chrono::system_clock> last_beat;

board b;

key up,down,left,right;

void update(double timeStep){
    
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    
    std::chrono::duration<float> duration = now-last_beat;
    std::chrono::duration<float> s = std::chrono::duration<float>(60.f/BPM);
    
    if(duration>s){
        last_beat = now;
    }
    
    
    
    
}
