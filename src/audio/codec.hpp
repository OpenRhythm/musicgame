#ifndef CODEC_HPP
#define CODEC_HPP

#include <string>

namespace FSaudio {

    /* This virtual class defines the structure of an audio decoding library.
     * It allows to support multiple audio formats.
     */

    class Codec {
    public:
        // The default constructor
        // @filename the absolute or relative file path
        Codec(const std::string filename)
        : filename(filename) {};

        // Gives info about the audio file
        // For now, only prints some info to Stdout
        // TODO return a structure ?
        virtual void getInfo() = 0;

        // Get the sample rate of the file
        virtual int getSampleRate() = 0;
        // Get the bit depth of the file
        virtual int getBitDepth() = 0;
        // Get the number of audio channels in the file
        virtual int getChannelCount() = 0;
        // Opens the audio file
        // Some heavy file access and decoding may occur here !
        // @return an error code TODO document and implement
        virtual int open() = 0;

        // Closes the file
        // TODO add a bool to check if the file is open/closed
        // @return an error code TODO document and implement
        virtual int close() = 0;

        // Writes into <buffer> up to <buffersize> bytes of decoded audio
        // TODO convert bytes to frames ?
        // @return the number of actually decodec bytes
        virtual int readBuffer(char* buffer, int bufferSize) = 0;

        // Returns the position inside the audio file
        // @return the value (in SOMETHING (seconds ?)) inside the audio file
        // TODO return a structure with more info ? (frame/time/total/…)
        virtual double getPosition() = 0;

    protected:
        // The filename (absolute or relative path)
        const std::string filename;
        bool validMusic;

        virtual void musicHasFinished() {};
        static bool song_is_playing;

        double position;
    };

}

#endif
