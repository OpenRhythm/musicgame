#include <soundio/soundio.h>

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
    Codec* mysong_soundio;

    static void write_callback(struct SoundIoOutStream *outstream,
            int frame_count_min, int frame_count_max) {
        const struct SoundIoChannelLayout *layout = &outstream->layout;
        struct SoundIoChannelArea *areas;
        int err;
        //std::cout << "max " << frame_count_max << std::endl;


        if ((err = soundio_outstream_begin_write(outstream, &areas, &frame_count_max))) {
            fprintf(stderr, "%s\n", soundio_strerror(err));
            exit(1);
        }

        int byte_count_max = frame_count_max* BYTES_PER_SAMPLE*CHANNELS_COUNT;
        int16_t data[byte_count_max];

        for (int bytes_decoded = 0; bytes_decoded < byte_count_max; ) {
            // Most decoders won't decode all bytes we need at once,
            // so we need multiple invocations !
            bytes_decoded += mysong_soundio->readBuffer(
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

    static void underflow_callback(struct SoundIoOutStream *outstream) {
        static int count = 0;
        fprintf(stderr, "underflow %d\n", count++);
    }


    int soundio_main(Codec *thesong) {
        mysong_soundio = thesong;
        int err;
        struct SoundIo *soundio = soundio_create();
        if (!soundio) {
            fprintf(stderr, "out of memory\n");
            return 1;
        }

        if ((err = soundio_connect(soundio))) {
            fprintf(stderr, "error connecting: %s", soundio_strerror(err));
            return 1;
        }

        soundio_flush_events(soundio);

        int default_out_device_index = soundio_default_output_device_index(soundio);
        if (default_out_device_index < 0) {
            fprintf(stderr, "no output device found");
            return 1;
        }

        struct SoundIoDevice *device = soundio_get_output_device(soundio, default_out_device_index);
        if (!device) {
            fprintf(stderr, "out of memory");
            return 1;
        }

        fprintf(stderr, "Output device: %s\n", device->name);

        struct SoundIoOutStream *outstream = soundio_outstream_create(device);
        outstream->format = SoundIoFormatS16NE;
        outstream->sample_rate = SAMPLE_RATE;
        outstream->write_callback = write_callback;
        outstream->underflow_callback = underflow_callback;
        outstream->software_latency = 0.01 ;

        if ((err = soundio_outstream_open(outstream))) {
            fprintf(stderr, "unable to open device: %s", soundio_strerror(err));
            return 1;
        }

        if (outstream->layout_error)
            fprintf(stderr, "unable to set channel layout: %s\n", soundio_strerror(outstream->layout_error));

        if ((err = soundio_outstream_start(outstream))) {
            fprintf(stderr, "unable to start device: %s", soundio_strerror(err));
            return 1;
        }

        for (;;)
            soundio_wait_events(soundio);

        soundio_outstream_destroy(outstream);
        soundio_device_unref(device);
        soundio_destroy(soundio);
        return 0;
    }
}
