#include "main.h"
#include "io/keyboard.h"
#include "io/mouse.h"
#include "audio/audio.h"

#include <iostream>
#include <chrono>

#include "game/beat-dungeon.h"

int BPM = 90;
constexpr int min_BPM =  60;
constexpr int max_BPM = 120;
int sub_beat = 3;// 16th notes, 3 means we start off on a full beat.

std::chrono::time_point<std::chrono::system_clock> last_beat;
std::chrono::time_point<std::chrono::system_clock> last_pulse;

board b;

key up,down,left,right;
mouse_t mouse;

bool pause_request = false;
bool unpause_request = false;
bool paused = false;

float volume = 1;
float volume_effects = 1;
float volume_music = 1;

std::chrono::time_point<std::chrono::system_clock> pause_time;

void update(double timeStep){
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    
    if(pause_request){
        paused = true;// seems simple, but if we have to do other stuff, we need a space to react appropriately.
        pause_request = false;
        pause_time = now;
    }
    
    if(unpause_request){
        paused = false;// seems simple, but if we have to do other stuff, we need a space to react appropriately.
        unpause_request = false;
        last_beat += now - pause_time;
    }
    
    if(paused) return;
    
    if(sub_beat<1||sub_beat>4) sub_beat = 3; // if we ever mistrack our beat, just snap back to our intended range.
    
    std::chrono::duration<float> duration = now-last_beat;
    
    // these could be more dynamic!!! (I could make a trueBPM variable & allow the sound track to speed up or slow down relative to the BPM.
    std::chrono::duration<float> full_beat          = std::chrono::duration<float>(60.f/BPM);
    std::chrono::duration<float> three_quarter_beat = std::chrono::duration<float>(45.f/BPM);
    std::chrono::duration<float> half_beat          = std::chrono::duration<float>(30.f/BPM);
    std::chrono::duration<float> quarter_beat       = std::chrono::duration<float>(15.f/BPM);
    
    static uint8_t movement = 0;
    
    std::chrono::duration<float> time_since_last_pulse = now - last_pulse;// (we are tracking 16th notes.
    
    if(time_since_last_pulse>std::chrono::duration<float>(7.5f/BPM)){// we ingore inputs for the first half of a 16 note. This is actually so the game feels more responsive.
        movement |= up.isActive()*1;
        movement |= left.isActive()*2;
        movement |= down.isActive()*4;
        movement |= right.isActive()*8;
    }
    
    if(duration>full_beat&&sub_beat==3){
        last_beat = now;
        last_pulse = now;
        
        b.step(4,movement);
        
        movement = 0;
        
        sub_beat = 4;
        
        return;
    }
    
    if(duration>three_quarter_beat&&sub_beat==2){
        last_pulse = now;
        
        b.step(3,movement);
        
        movement = 0;
        
        sub_beat = 3;
        
        return;
    }
    
    if(duration>half_beat&&sub_beat==1){
        last_pulse = now;
        
        b.step(2,movement);
        
        movement = 0;
        
        sub_beat = 2;
        
        return;
    }
    
    if(duration>quarter_beat&&sub_beat==4){
        last_pulse = now;
        
        b.step(1,movement);
        
        movement = 0;
        
        sub_beat = 1;
        
        return;
    }
    
}
