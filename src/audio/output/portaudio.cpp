#include <stdio.h>
#include <iostream>
#include <portaudio.h>
#include "portaudio.hpp"

namespace FSaudio {


    #define NUM_SECONDS   (500)
    #define SAMPLE_RATE   (44100)
    #define FRAMES_PER_BUFFER  (64)

    Codec* mysong;
    /* This routine will be called by the PortAudio engine when audio is needed.
    ** It may called at interrupt level on some machines so don't do anything
    ** that could mess up the system like calling malloc() or free().
    */
    static int patestCallback(  const void *inputBuffer, void *outputBuffer,
                                unsigned long framesPerBuffer,
                                const PaStreamCallbackTimeInfo* timeInfo,
                                PaStreamCallbackFlags statusFlags,
                                void *userData ) {
        // Prevent unused variable warnings.
        (void) timeInfo;
        (void) statusFlags;
        (void) inputBuffer;

        std::cout << "asking  " << 4*framesPerBuffer << " bytes (" << framesPerBuffer << " frames)" << std::endl;

        int decoded_bytes = mysong->readBuffer((char*)outputBuffer, 4*framesPerBuffer);

        std::cout << "decoded " << decoded_bytes     << " bytes (" << decoded_bytes/4 << " frames)" << std::endl << std::endl;

        return paContinue;
    }

    // This routine is called by portaudio when playback is done.
    static void StreamFinished(void* userData) {
        printf( "Stream Completed.");
    }


    int pa_main(Codec* thesong) {
        mysong = thesong;

        PaStreamParameters outputParameters;
        PaStream *stream;
        PaError err;

        printf("PortAudio Test: output sine wave. SR = %d, BufSize = %d\n", SAMPLE_RATE, FRAMES_PER_BUFFER);

        err = Pa_Initialize();
        if( err != paNoError ) goto error;

        outputParameters.device = Pa_GetDefaultOutputDevice(); // default output device
        if (outputParameters.device == paNoDevice) {
            fprintf(stderr,"Error: No default output device.\n");
            goto error;
        }
        outputParameters.channelCount = 2;       // stereo output
        outputParameters.sampleFormat = paInt16; // 32 bit floating point output
        outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
        outputParameters.hostApiSpecificStreamInfo = NULL;

        err = Pa_OpenStream(
                &stream,
                NULL, // no input
                &outputParameters,
                SAMPLE_RATE,
                FRAMES_PER_BUFFER,
                paClipOff,      // we won't output out of range samples so don't bother clipping them
                patestCallback,
                NULL);
        if( err != paNoError ) goto error;

        err = Pa_SetStreamFinishedCallback( stream, &StreamFinished );
        if( err != paNoError ) goto error;

        err = Pa_StartStream( stream );
        if( err != paNoError ) goto error;

        printf("Play for %d seconds.\n", NUM_SECONDS );
        Pa_Sleep( NUM_SECONDS * 1000 );

        err = Pa_StopStream( stream );
        if( err != paNoError ) goto error;

        err = Pa_CloseStream( stream );
        if( err != paNoError ) goto error;

        Pa_Terminate();
        printf("Test finished.\n");

        return err;
    error:
        Pa_Terminate();
        fprintf( stderr, "An error occured while using the portaudio stream\n" );
        fprintf( stderr, "Error number: %d\n", err );
        fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
        return err;
    }

}
