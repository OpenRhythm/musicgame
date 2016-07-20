#include "vorbis.hpp"

#include <stdlib.h>
#include <iostream>


int VorbisSong::open() {
    myFile = fopen(this->filename.c_str(), "rb");
    if (!myFile)
        return -1;

    int open_ret = ov_open_callbacks(myFile, &myVorbisFile, NULL, 0, OV_CALLBACKS_NOCLOSE);

    switch (open_ret) {
        case OV_EREAD:       // A read from media returned an error.
        case OV_ENOTVORBIS:  // Bitstream does not contain any Vorbis data.
        case OV_EVERSION:    // Vorbis version mismatch.
        case OV_EBADHEADER:  // Invalid Vorbis bitstream header.
        case OV_EFAULT:      // Internal logic fault; indicates a bug or heap/stack corruption.
            fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");
            fclose(myFile);
            return 1;
            break;
        case 0:
            break;
    }

    if (ov_pcm_seek(&myVorbisFile, 0) != 0)  // This is because some files do not seek to 0 automatically
        fprintf(stderr, "Error seeking file to position 0.\n");
    else
        fprintf(stdout, "Seeked file to position 0.\n");
    return 0;
}

int VorbisSong::close() {
    ov_clear(&myVorbisFile);
}

void VorbisSong::getInfo() {
    char **ptr=ov_comment(&myVorbisFile,-1)->user_comments;
    vorbis_info *vi=ov_info(&myVorbisFile,-1);
    while(*ptr){
      fprintf(stderr,"%s\n",*ptr);
      ++ptr;
    }
    fprintf(stderr,"\nBitstream is %d channel, %ldHz\n",vi->channels,vi->rate);
    fprintf(stderr,"\nDecoded length: %ld samples\n",
            (long)ov_pcm_total(&myVorbisFile,-1));
    fprintf(stderr,"Encoded by: %s\n\n",ov_comment(&myVorbisFile,-1)->vendor);

}

int VorbisSong::readBuffer(char* buffer, int bufferSize) {
    long read_ret = ov_read(&myVorbisFile, buffer, bufferSize,
        0, 2, 1, &current_section);

    switch(read_ret) {
        case 0:         // indicates EOF
            eof = 1;
            std::cout << "auie" << std::endl;
            return 1;
            break;
        case OV_HOLE:
                // indicates there was an interruption in the data.
                // (one of: garbage between pages, loss of sync followed by recapture, or a corrupt page)
        case OV_EBADLINK:
                // indicates that an invalid stream section was supplied to libvorbisfile, or the requested link is corrupt.
        case OV_EINVAL:
                // indicates the initial file headers couldn't be read or are corrupt, or that the initial open call for vf failed.
            // There was an error, TL;DR
            std::cout << "auie" << std::endl;
            break;
        default:
            return 0;
    }
}

