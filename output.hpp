#ifndef OUTPUT_HPP
#define OUTPUT_HPP

namespace FSaudio {

    /* This virtual class defines the structure of an audio output stream.
     * It allows to support multiple audio backends if needed.
     * TODO we will only use one output device at a time, it should be checked.
     */

    class OutputStream {
    public:
        OutputStream() {};

        virtual void getInfo() = 0;
        virtual int open() = 0;
        virtual int close() = 0;

        virtual int readBuffer(char* buffer, int bufferSize) = 0;

        // Useful ?
        virtual double getPosition() {};

        bool setSampleRate(int sr);
        bool setChannelCount(int cc);

    protected:
        int sampleRate;
        int channelCount;

    };

}


#endif
