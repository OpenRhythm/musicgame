// Copyright (c) 2015-2017 Matthew Sitton <matthewsitton@gmail.com>
// See LICENSE in the project root for license information.

#pragma once
#include <string>

#include <vorbis/vorbisfile.h>

#include "streams.hpp"

namespace ORCore
{
    class VorbisSource: public ProducerStream
    {
    public:
        VorbisSource(std::string filename);
        ~VorbisSource();

        StreamFormat get_format();
        void do_pause();
        void pull(Buffer& buffer);
        void seek(double time);
        double get_length();

    private:
        std::string m_filename;
        std::atomic<double> m_timeSeek;

        // only written once before threads come time play, so no need to be atomic.
        double m_timeLength; 

        OggVorbis_File m_vorbisFile;
        vorbis_info *m_info;
    };
}