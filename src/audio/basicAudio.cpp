#include <iostream>

#include "basicAudio.hpp"

Song::Song(const std::string filename)
: filename(filename),
  validMusic(false) {
    music = Mix_LoadMUS(this->filename.data());
    if (!music) {
        cerr << "Could not load the music : "
             << Mix_GetError()
             << endl;
    }
    validMusic = true;

    Mix_HookMusicFinished(*musicHasFinished);
}

bool Song::song_is_playing = false;

bool Song::isPlaying(){
    return song_is_playing;
}


void Song::play(){
    if (Mix_PlayMusic(
            music,
            1) == -1) {
        cerr << "Could not play the music : "
             << Mix_GetError()
             << endl;
    }
    song_is_playing = true;
}

void Song::playWithFadeIn(double fadeInLength, double timeAtFadeInStart){
    cout << "playwithfadein" << fadeInLength*1000 << endl;
    if (Mix_FadeInMusicPos(
            music,
            1,          // Repetition count
            fadeInLength*1000,
            timeAtFadeInStart) == -1) {
        cerr << "Could not play the music : "
             << Mix_GetError()
             << endl;
    }
    song_is_playing = true;
}

void Song::pause(){
    Mix_PauseMusic();
    song_is_playing = false;
}

void Song::pauseWithFadeOut(double fadeOutLength){
    cout << "startFadeOut" << endl;
    Mix_FadeOutMusic(2000);
}


void Song::musicHasFinished() {
    cout << "musicHasFinished" << endl;
    song_is_playing = false;
}


void Song::setOnMusicFinishedCallback(void (*music_finished)(void)){

}




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

    printf("Average volume is %d\n",Mix_Volume(-1,-1));
}

