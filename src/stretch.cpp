#include <cstring>
#include "stretch.h"
#include "./signalsmith-stretch/signalsmith-stretch.h"

struct stretch_t {
    int channels;
    float sampleRate;
    signalsmith::stretch::SignalsmithStretch<float>* stretch;
};

struct View
{
    float* data;
    int channel;
    int stride;
    
    View(float* data, int channel, int stride)
        : data(data), channel(channel), stride(stride) {}

    float& operator[](int offset) {
        return data[(offset * stride) + channel];
    }

    const float& operator[](int offset) const {
        return data[(offset * stride) + channel];
    }
};

class InterleavedBuffer {
    float* data;
    int channel;

public:
    InterleavedBuffer(float* data, int channel) : data(data), channel(channel) {}
    InterleavedBuffer(const float* data, int channel) : data(const_cast<float*>(data)), channel(channel) {}

    View operator[](int c) {
        return View(data, c, channel);
    }
};

stretch_t* stretch_create() {
    stretch_t* s = new stretch_t();
    s->stretch = new signalsmith::stretch::SignalsmithStretch<float>();
    return s;
}

stretch_t* stretch_create_seed(long seed) {
    stretch_t* s = new stretch_t();
    s->stretch = new signalsmith::stretch::SignalsmithStretch<float>(seed);
    return s;
}

void stretch_delete(stretch_t* stretch) {
    delete stretch->stretch;
    memset(stretch, 0xFF, sizeof(stretch_t));
    delete stretch;
}

void stretch_preset_default(stretch_t* stretch, int nChannels, float sampleRate, bool splitComputation) {
    stretch->stretch->presetDefault(nChannels, sampleRate, splitComputation);
    stretch->channels = nChannels;
    stretch->sampleRate = sampleRate;
}

void stretch_preset_cheaper(stretch_t* stretch, int nChannels, float sampleRate, bool splitComputation) {
    stretch->stretch->presetCheaper(nChannels, sampleRate, splitComputation);
    stretch->channels = nChannels;
    stretch->sampleRate = sampleRate;
}

void stretch_configure(stretch_t* stretch, int nChannels, int blockSamples, int intervalSamples, bool splitComputation) {
    stretch->stretch->configure(nChannels, blockSamples, intervalSamples, splitComputation);
}

void stretch_reset(stretch_t* stretch) {
    stretch->stretch->reset();
}

int stretch_input_latency(stretch_t* stretch) {
    return stretch->stretch->inputLatency();
}

int stretch_output_latency(stretch_t* stretch) {
    return stretch->stretch->outputLatency();
}

int stretch_block_samples(stretch_t* stretch) {
    return stretch->stretch->blockSamples();
}

int stretch_interval_samples(stretch_t* stretch) {
    return stretch->stretch->intervalSamples();
}

bool stretch_split_computation(stretch_t* stretch) {
    return stretch->stretch->splitComputation();
}

void stretch_set_transpose_semitones(stretch_t* stretch, float semitones, float tonalityLimit) {
    stretch->stretch->setTransposeSemitones(semitones, tonalityLimit);
}

void stretch_set_transpose_factor(stretch_t* stretch, float factor, float tonalityLimit) {
    stretch->stretch->setTransposeFactor(factor, tonalityLimit);
}

void stretch_set_freq_map(stretch_t* stretch, float (*inputToOutput)(float)) {
    stretch->stretch->setFreqMap(inputToOutput);
}

void stretch_set_formant_factor(stretch_t* stretch, float multiplier, bool compensatePitch) {
    stretch->stretch->setFormantFactor(multiplier, compensatePitch);
}

void stretch_set_formant_semitones(stretch_t* stretch, float semitones, bool compensatePitch) {
    stretch->stretch->setFormantSemitones(semitones, compensatePitch);
}

void stretch_set_formant_base(stretch_t* stretch, float baseFreq) {
    stretch->stretch->setFormantBase(baseFreq);
}

void stretch_seek(stretch_t* stretch, const float* input, int inputSamples, double playbackRate) {
    InterleavedBuffer inBuffer(input, stretch->channels);
    stretch->stretch->seek(inBuffer, inputSamples, playbackRate);
}

void stretch_flush(stretch_t* stretch, float* output, int pcmOutLength, double playbackRate) {
    InterleavedBuffer outBuffer(output, stretch->channels);
    stretch->stretch->flush(outBuffer, pcmOutLength, playbackRate);
}

int stretch_seek_length(stretch_t* stretch) {
    return stretch->stretch->seekLength();
}

void stretch_output_seek(stretch_t* stretch, const float* input, int inputLength) {
    InterleavedBuffer inBuffer(input, stretch->channels);
    stretch->stretch->outputSeek(inBuffer, inputLength);
}

int stretch_output_seek_length(stretch_t* stretch, float playbackRate) {
    return stretch->stretch->outputSeekLength(playbackRate);
}

void stretch_process(stretch_t* stretch, const float* input, int pcmLength, float* output, int pcmOutLength) {
    InterleavedBuffer inBuffer(input, stretch->channels);
    InterleavedBuffer outBuffer(output, stretch->channels);
    stretch->stretch->process(inBuffer, pcmLength, outBuffer, pcmOutLength);
}

bool stretch_exact(stretch_t* stretch, const float* input, int pcmLength, float* output, int pcmOutLength) {
    InterleavedBuffer inBuffer(input, stretch->channels);
    InterleavedBuffer outBuffer(output, stretch->channels);
    return stretch->stretch->exact(inBuffer, pcmLength, outBuffer, pcmOutLength);
}