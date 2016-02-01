#ifndef BASICAUDIO_HPP
#define BASICAUDIO_HPP

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
bool is_audio_playing();

void startFadeOutMusic();
void fadeOutHasFinished();


#endif  // BASICAUDIO_HPP
