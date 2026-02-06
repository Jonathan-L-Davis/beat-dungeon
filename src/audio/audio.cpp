#include "audio.h"
#include "main.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>
#include <map>
#include <vector>
#include <mutex>

#include "util/tinywav/tinywav.h"

SDL_AudioSpec spec;
SDL_AudioStream *stream = NULL;

float *audio_buffer;

constexpr int sample_rate = 24000;//44100;
int idx = 0;

uint32_t metronome_sound_size;
uint8_t* metronome_sound_data;

std::mutex M;

struct audio_index{
    std::string instrument;
    uint32_t idx;
    float volume;
};

std::vector<audio_index> audio_indices;
std::map<std::string,std::vector<float>> instruments;

void audio_func(){
    
    while(!quit){
        constexpr int minimum_audio = 512;
        if (SDL_GetAudioStreamQueued(stream) < minimum_audio) {
            std::scoped_lock L(M);
            static float samples[512];  /* this will feed 512 samples each frame until we get to our maximum. */
            
            std::memset(samples,0,512*4);
            
            for(audio_index& index : audio_indices ){
                for(int i = 0; i < 512; i++){
                    
                    const std::vector<float>& instrument = instruments[index.instrument];
                    
                    samples[i] += index.volume*instrument[index.idx];
                    index.idx += 1;
                    
                    if( index.idx>=instrument.size() ){
                        break;
                    }
                }
            }
            
            // erases any sound that has finished playing. This way we don't play instruments for too long/access bad memory.
            for(int i = ((int)audio_indices.size())-1; i >= 0;  i--){
                const std::vector<float>& instrument = instruments[audio_indices[i].instrument];
                
                if( audio_indices[i].idx>=instrument.size() ){
                    audio_indices.erase(audio_indices.begin()+i);
                }
            }
            /* feed the new data to the stream. It will queue at the end, and trickle out as the hardware needs more data. */
            SDL_PutAudioStreamData(stream, samples, 4*512);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
}

void populate_sound_vec(std::vector<float>& sound, std::string file_name){
    
    TinyWav metronome_wav;
    tinywav_open_read(&metronome_wav, "res/audio/metronome.wav", TW_SPLIT);
    
    int block_size = 20000;
    float *samples = new float[2*block_size];
    
    float* samplePtrs[2];
    for (int j = 0; j < 2; ++j) {
        samplePtrs[j] = samples + j*block_size;
    }
    
    tinywav_read_f(&metronome_wav, samplePtrs, block_size);
    
    for(int i = 0; i < block_size; i++){
        sound.push_back(samplePtrs[0][i]);
    }
    
    tinywav_close_read(&metronome_wav);
}

void init_sound(){
    if (!SDL_Init(SDL_INIT_AUDIO))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return;
    }
    
    spec.channels = 1;
    spec.format = SDL_AUDIO_F32;
    spec.freq = sample_rate;
    
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    SDL_ResumeAudioStreamDevice(stream);
    
    std::vector<float> metronome;
    populate_sound_vec(metronome,"res/audio/metronome.wav");
    
    instruments["metronome"] = metronome;
    
}

void play_sound(std::string sound_name, float volume){
    std::scoped_lock L(M);
    audio_indices.push_back({sound_name,0,volume});
}
