#ifndef MAIN_H
#define MAIN_H

#include "common.h"

#include <chrono>
#include <thread>

void init(std::string);

void input();
void update(double);
void draw();

void terminate();

struct app_timer{


    void start(){
        lastTime = std::chrono::steady_clock::now();
    }

    void accumulate(){
        timeLeft += std::chrono::steady_clock::now()-lastTime;
        lastTime = std::chrono::steady_clock::now();
    }

    bool need_update(){
        if( timeLeft > timeStep ){
            timeLeft-=timeStep;
            return true;
        }
        return false;
    }

    bool need_frame(){
        if( std::chrono::steady_clock::now()-lastFrame>=MS_PER_FRAME ){
            lastFrame = std::chrono::steady_clock::now();
            return true;
        }
        return false;
    }

    void sleep(){
        std::chrono::duration<double> timeTillNextFrame = MS_PER_FRAME - std::chrono::steady_clock::now().time_since_epoch() + lastFrame.time_since_epoch()-std::chrono::milliseconds(1);
        if(timeTillNextFrame>=MS_PER_FRAME) timeTillNextFrame = std::chrono::seconds(0);
            std::this_thread::sleep_for( timeTillNextFrame );
    }

    double time_step(){
        return timeStep.count();
    }

    double TARGET_FPS = 60;//not a time, is a target number/scalar
    std::chrono::duration<double> MS_PER_FRAME;//---duration
    double UPDATES_PER_SECOND = 100;//not a time, is a target number/scalar
    std::chrono::duration<double> timeStep;//---duration

    std::chrono::duration<double> timeLeft;
    std::chrono::time_point<std::chrono::steady_clock> lastTime;

    std::chrono::time_point<std::chrono::steady_clock> lastFrame;
    std::chrono::time_point<std::chrono::steady_clock> startTime;

    app_timer(){
        MS_PER_FRAME = std::chrono::duration< double, std::ratio<1,1000> > {1000.0/TARGET_FPS};
        timeStep = std::chrono::duration< double, std::ratio<1,1000> > {1000.0/UPDATES_PER_SECOND};
    }

};

extern bool quit;
extern app_timer app_clock;

#endif//MAIN_H
