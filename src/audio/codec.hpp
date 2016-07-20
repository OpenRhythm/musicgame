#ifndef CODEC_HPP
#define CODEC_HPP

#include <string>

// Virtual class to declare decoders structure

class Codec {
public:
    Codec(const std::string filename)
    : filename(filename) {};

    virtual void getInfo() = 0;
    virtual int open() = 0;
    virtual int close() = 0;

    virtual int readBuffer(char* buffer, int bufferSize) = 0;

    // Useful ?
    virtual double getPosition() {};

protected:
    const std::string filename;
    bool validMusic;

    virtual void musicHasFinished() {};
    static bool song_is_playing;

    double position;
};

#endif
