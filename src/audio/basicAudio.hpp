#ifndef BASICAUDIO_HPP
#define BASICAUDIO_HPP

#include <string.h>

#include <SDL_mixer.h>

#define SDL2MIXER_FLAGS     MIX_INIT_OGG|MIX_INIT_FLAC
#define SDL2MIXER_FREQUENCY 44100
#define SDL2MIXER_FORMAT    MIX_DEFAULT_FORMAT
#define SDL2MIXER_CHANNEL   2
#define SDL2MIXER_CHUNKSIZE 1024

#define OggTestFile "MusicExamples/Kamelot - Nights of Arabia/guitar.ogg"

using std::cout;
using std::cerr;
using std::endl;

int    init_audio();
void deinit_audio();

void test_audio();


class Song {
public:
    Song(const std::string filename);

    bool isPlaying();

    void play();
    void playWithFadeIn(double fadeInLength, double timeAtFadeStart);
    void pause();
    void pauseWithFadeOut(double fadeOutLength);


    void setOnMusicFinishedCallback(void (*music_finished)(void));

protected:
    const std::string filename;
    Mix_Music* music;
    bool validMusic;

    static void musicHasFinished();
    static bool song_is_playing;

    double position;




};

#endif  // BASICAUDIO_HPP
