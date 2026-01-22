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

board b;

key up,down,left,right;
mouse_t mouse;

bool pause_request = false;
bool unpause_request = false;
bool paused = true;

std::chrono::time_point<std::chrono::system_clock> pause_time;

void update(double timeStep){
    play_sound("res/audio/metronome-85688.mp3");
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
    
    movement |= up.isActive()*1;
    movement |= left.isActive()*2;
    movement |= down.isActive()*4;
    movement |= right.isActive()*8;
    
    if(duration>full_beat&&sub_beat==3){
        last_beat = now;
        b.step(4,movement);
        
        movement = 0;
        
        sub_beat = 4;
        
        play_sound("res/audio/metronome-85688.mp3");
        
        return;
    }
    
    if(duration>three_quarter_beat&&sub_beat==2){
        b.step(3,movement);
        
        movement = 0;
        
        sub_beat = 3;
        
        return;
    }
    
    if(duration>half_beat&&sub_beat==1){
        b.step(2,movement);
        
        movement = 0;
        
        sub_beat = 2;
        
        return;
    }
    
    if(duration>quarter_beat&&sub_beat==4){
        b.step(1,movement);
        
        movement = 0;
        
        sub_beat = 1;
        
        return;
    }
    
}
