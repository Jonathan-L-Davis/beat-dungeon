#include "audio.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_audio.h>

SDL_AudioSpec spec;
SDL_AudioStream *stream = NULL;

float *audio_buffer;
static int current_sine_sample = 0;
constexpr int sample_rate = 44100;

void init_sound(){
    if (!SDL_Init(SDL_INIT_AUDIO))
    {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return;
    }
    
    spec.channels = 1;
    spec.format = SDL_AUDIO_F32;
    spec.freq = 44100;
    
    stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
    SDL_ResumeAudioStreamDevice(stream);
}

void play_sound(std::string file_name){
    const int minimum_audio = (sample_rate * sizeof (float)) / 2;  /* 8000 float samples per second. Half of that. */
    if (SDL_GetAudioStreamQueued(stream) < minimum_audio) {
        static float samples[512];  /* this will feed 512 samples each frame until we get to our maximum. */
        int i;

        /* generate a 440Hz pure tone */
        for (i = 0; i < SDL_arraysize(samples); i++) {
            const int freq = 220;
            const float phase = current_sine_sample * freq / float(sample_rate);
            samples[i] = 0*SDL_sinf(phase * 2 * SDL_PI_F);
            current_sine_sample++;
        }

        /* wrapping around to avoid floating-point errors */
        current_sine_sample %= sample_rate;

        /* feed the new data to the stream. It will queue at the end, and trickle out as the hardware needs more data. */
        SDL_PutAudioStreamData(stream, samples, 4*512);
    }
}
