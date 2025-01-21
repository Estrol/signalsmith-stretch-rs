
#include "wrapper.h"

#define SIGNALSMITH_STRETCH_IMPLEMENTATION
#include <signalsmith-stretch.h>

#include <stddef.h>
#include <vector>
#include <cstring>

struct signalsmith_stretch {
    signalsmith::stretch::SignalsmithStretch instance;
    int channel_count;
};

signalsmith_stretch_t *signalsmith_stretch_create(int channel_count, size_t block_length, size_t interval) {
    auto handle = new signalsmith_stretch;
    handle->channel_count = channel_count;
    handle->instance.configure(channel_count, block_length, interval);

    return handle;
}

signalsmith_stretch_t *signalsmith_stretch_create_preset_default(int channel_count, float sample_rate) {
    auto handle = new signalsmith_stretch;
    handle->channel_count = channel_count;
    handle->instance.presetDefault(channel_count, sample_rate);

    return handle;
}

signalsmith_stretch_t *signalsmith_stretch_create_preset_cheaper(int channel_count, float sample_rate) {
    auto handle = new signalsmith_stretch;
    handle->channel_count = channel_count;
    handle->instance.presetCheaper(channel_count, sample_rate);

    return handle;
}

void signalsmith_stretch_destroy(signalsmith_stretch_t *handle) {
    delete handle;
}

void signalsmith_stretch_reset(signalsmith_stretch_t *handle) {
    return handle->instance.reset();
}

size_t signalsmith_stretch_input_latency(signalsmith_stretch_t *handle) {
    return static_cast<size_t>(handle->instance.inputLatency());
}

size_t signalsmith_stretch_output_latency(signalsmith_stretch_t *handle) {
    return static_cast<size_t>(handle->instance.outputLatency());
}

void signalsmith_stretch_set_transpose_factor(signalsmith_stretch_t *handle, float multiplier, float tonality_limit) {
    handle->instance.setTransposeFactor(multiplier, tonality_limit);
}

void signalsmith_stretch_set_transpose_factor_semitones(signalsmith_stretch_t *handle, float semitones, float tonality_limit) {
    handle->instance.setTransposeSemitones(semitones, tonality_limit);
}

void signalsmith_stretch_seek(signalsmith_stretch_t *handle, float *input, size_t input_length, double playback_rate) {
    handle->instance.seek(input, input_length, playback_rate);
}

void signalsmith_stretch_process(signalsmith_stretch_t *handle,
                                 float *input, size_t input_length,
                                 float *output, size_t output_length) {
    handle->instance.process(input, input_length, output, output_length);
}

void signalsmith_stretch_flush(signalsmith_stretch_t *handle,
                               float *output, size_t output_length) {
    handle->instance.flush(output, output_length);
}
