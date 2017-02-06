#ifndef VORBIS_HPP
#define VORBIS_HPP

#include <string>
#include <vorbis/vorbisfile.h>

#include "audio/codec.hpp"

#ifdef _WIN32
#   include <io.h>
#   include <fcntl.h>
#endif

namespace FSaudio {

    class VorbisSong: public Codec {
    public:
        VorbisSong(const std::string filename)
        : Codec(filename) {};

        // @inherit
        void getInfo();
        // @inherit
        virtual int getSampleRate();
        // @inherit
        virtual int getBitDepth() { return 16; };
        // @inherit
        virtual int getChannelCount();
        // @inherit
        virtual double getPosition();
        // @inherit
        int open();
        // @inherit
        int close();

        int readBuffer(char* buffer, int bufferSize);


    protected:
        FILE *myFile;
        OggVorbis_File myVorbisFile;
        int eof=0;
        int current_section = 0;
    };

}

#endif  // VORBIS_HPP
