
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <iostream>

#include "soundio.hpp"

// TODO those defines should be runtime variables, dependant of the stream

// file dependant
#define SAMPLE_RATE 44100
// file dependant (now : uint16_t)
#define BYTES_PER_SAMPLE 2
// file dependent (now : stereo)
#define CHANNELS_COUNT 2

namespace FSaudio {
// The global soundio instance
struct SoundIo      *soundio = NULL;
// The default device instance
struct SoundIoDevice *device = NULL;

int soundio_initialize() {
    if (soundio) // Already initialized
        return 0;

    soundio = soundio_create();
    if (!soundio) {
        std::cerr << "SoundIO_create: out of memory" << std::endl;
        return 1;
    }

    soundio->app_name = APP_NAME;

    int err = soundio_connect(soundio);
    if (err) {
        std::cerr << "SoundIO_connect error: "
                  << soundio_strerror(err)
                  << std::endl;
        return 1;
    }

    soundio_flush_events(soundio);
    return 0;
}

int soundio_quit() {
    soundio_destroy(soundio);
    return 0;
}

int soundio_connect_default_output_device() {
    if (device) // Already initialized
        return 0;

    int default_out_device_index = soundio_default_output_device_index(soundio);
    if (default_out_device_index < 0) {
        std::cerr << "SoundIO: No output device found"
                  << std::endl;
        return 1;
    }

    device = soundio_get_output_device(soundio, default_out_device_index);
    if (!device) {
        std::cerr << "SoundIO: out of memory" << std::endl;
        return 1;
    }

    return 0;
}

int soundio_disconnect_device() {
    soundio_device_unref(device);
    return 0;
}


void SoundIoOStream::write_callback_static(
    struct SoundIoOutStream *outstream,
    int frame_count_min, int frame_count_max) {
    return ((SoundIoOStream*)outstream->userdata)
        ->write_callback(outstream, frame_count_min, frame_count_max);
}

void SoundIoOStream::write_callback(
    struct SoundIoOutStream *outstream,
    int frame_count_min, int frame_count_max) {
    const struct SoundIoChannelLayout *layout = &outstream->layout;
    struct SoundIoChannelArea *areas;
    int err;

    if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count_max))) {
        fprintf(stderr, "%s\n", soundio_strerror(err));
        exit(1);
    }

    int byte_count_max = frame_count_max* BYTES_PER_SAMPLE*CHANNELS_COUNT;
    int16_t data[byte_count_max];

    for (int bytes_decoded = 0; bytes_decoded < byte_count_max; ) {
        // Most decoders won't decode all bytes we need at once,
        // so we need multiple invocations !
        bytes_decoded += theSong->readBuffer(
            ((char*)data)+ bytes_decoded, byte_count_max - bytes_decoded);
    }

    // Now we copy the data into the outstream !
    for (int i = 0; i < frame_count_max; ++i) {
        for (int channel = 0; channel < layout->channel_count; ++channel) {
            int16_t sample = data[channel + 2*i];
            int16_t *ptr = (int16_t*)(areas[channel].ptr + areas[channel].step * i);
            *ptr = sample;
        }
    }

    if ((err = soundio_outstream_end_write(outstream))) {
        fprintf(stderr, "soundio erreur : %s\n", soundio_strerror(err));
        exit(1);
    }
}

void SoundIoOStream::underflow_callback_static(struct SoundIoOutStream *outstream) {
    return ((SoundIoOStream*)outstream->userdata)->underflow_callback(outstream);
}
void SoundIoOStream::underflow_callback(struct SoundIoOutStream *outstream) {
    static int count = 0;
    fprintf(stderr, "underflow %d\n", count++);
}


int SoundIoOStream::setInput(Codec *theSong) {
    this->theSong = theSong;
    return 0;
}

int SoundIoOStream::open(double latency) {
    SoundIoFormat format;
    switch(theSong->getBitDepth()) {
        case  8: format = SoundIoFormatS8;
        case 16: format = SoundIoFormatS16NE;
        case 24: format = SoundIoFormatS24NE;
        case 32: format = SoundIoFormatS32NE;
        default: format = SoundIoFormatS16NE;

    }
    return open(format, theSong->getSampleRate(), latency);
}

int SoundIoOStream::open(SoundIoFormat format, int sample_rate, double latency) {
    outstream = soundio_outstream_create(device);
    outstream->format           = format;
    outstream->sample_rate      = sample_rate;
    outstream->software_latency = latency;

    outstream->userdata             = this;
    outstream->write_callback       = &SoundIoOStream::write_callback_static;
    outstream->underflow_callback   = &SoundIoOStream::underflow_callback_static;

    int err = soundio_outstream_open(outstream);
    if (err) {
        std::cerr << "unable to open device: "
                    << soundio_strerror(err) << std::endl;
        return 1;
    }

    err = outstream->layout_error;
    if (err) {
        std::cerr << "unable to set channel layout: "
                    << soundio_strerror(err) << std::endl;
        return 1;
    }

    err = soundio_outstream_start(outstream);
    if (err) {
        std::cerr << "unable to start device: "
                    << soundio_strerror(err) << std::endl;
        return 1;
    }
    return 0;
}

void SoundIoOStream::close() {
    soundio_outstream_destroy(outstream);
}

int soundio_main(Codec *thesong) {

    soundio_initialize();
    soundio_connect_default_output_device();
    SoundIoOStream *mysoundioostream = new SoundIoOStream();
    mysoundioostream->setInput(thesong);
    mysoundioostream->open(0.01);

    for (;;)
        soundio_wait_events(soundio);

    delete(mysoundioostream);
    soundio_disconnect_device();
    soundio_quit();
    return 0;
}


}
