#include <iostream>
#include <SDL_mixer.h>

#include "basicAudio.hpp"

bool audio_is_playing = false;
Mix_Music* music;

int init_audio() {
    int initted=Mix_Init(SDL2MIXER_FLAGS);
    if((initted & (SDL2MIXER_FLAGS) ) != (SDL2MIXER_FLAGS)) {
        cerr << "Mix_Init: Failed to init required flac, ogg and mod support!" << endl;
        cerr << "Mix_Init: " << Mix_GetError() << endl;
        return 1;
    }

    // open 44.1KHz, signed 16bit, system byte order,
    //      stereo audio, using 1024 byte chunks
    if(Mix_OpenAudio(SDL2MIXER_FREQUENCY,
                     SDL2MIXER_FORMAT,
                     SDL2MIXER_CHANNEL,
                     SDL2MIXER_CHUNKSIZE)==-1) {
        cerr << "Mix_OpenAudio: " << Mix_GetError() << endl;
        return 1;
    }

        cout << "Mix initialized without errors"
                  << endl;
    return 0;
}

void deinit_audio() {
    Mix_CloseAudio();
    Mix_Quit();
}



void test_audio() {
    // print the number of sample chunk decoders available
    printf("There are %d sample chunk deocoders available\n", Mix_GetNumChunkDecoders());

    // print sample chunk decoders available
    int i,max=Mix_GetNumChunkDecoders();
    for(i=0; i<max; ++i)
        cout << "Sample chunk decoder " << i
                  << " is for " << Mix_GetChunkDecoder(i)
                  << endl;

    music = Mix_LoadMUS(OggTestFile);
    if(!music) {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
        // handle error
    }
    printf("Average volume is %d\n",Mix_Volume(-1,-1));

    Mix_HookMusicFinished(*fadeOutHasFinished);

    if(Mix_FadeInMusicPos(music, -1, 2000,10)==-1) {
        printf("Mix_PlayMusic: %s\n", Mix_GetError());
        // well, there's no music, but most games don't break without music...
    }
    audio_is_playing = true;
}

void startFadeOutMusic() {
    cout << "startFadeOutMusic" << endl;
    Mix_FadeOutMusic(2000);
}

void fadeOutHasFinished() {
    cout << "fadeOutHasFinished" << endl;
    audio_is_playing = false;
}

bool is_audio_playing() {
    return audio_is_playing;
}
