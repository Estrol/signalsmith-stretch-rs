#ifndef SIGNALSMITH_STRETCH_WRAPPER_H
#define SIGNALSMITH_STRETCH_WRAPPER_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct stretch_t stretch_t;

stretch_t* stretch_create();

stretch_t* stretch_create_seed(long seed);

void stretch_delete(stretch_t* stretch);

void stretch_preset_default(stretch_t* stretch, int nChannels, float sampleRate, bool splitComputation);

void stretch_preset_cheaper(stretch_t* stretch, int nChannels, float sampleRate, bool splitComputation);

void stretch_configure(stretch_t* stretch, int nChannels, int blockSamples, int intervalSamples, bool splitComputation);

void stretch_reset(stretch_t* stretch);

int stretch_input_latency(stretch_t* stretch);

int stretch_output_latency(stretch_t* stretch);

int stretch_block_samples(stretch_t* stretch);

int stretch_interval_samples(stretch_t* stretch);

bool stretch_split_computation(stretch_t* stretch);

void stretch_set_transpose_semitones(stretch_t* stretch, float semitones, float tonalityLimit);

void stretch_set_transpose_factor(stretch_t* stretch, float factor, float tonalityLimit);

void stretch_set_freq_map(stretch_t* stretch, float (*inputToOutput)(float));

void stretch_set_formant_factor(stretch_t* stretch, float multiplier, bool compensatePitch);

void stretch_set_formant_semitones(stretch_t* stretch, float semitones, bool compensatePitch);

void stretch_set_formant_base(stretch_t* stretch, float baseFreq);

void stretch_seek(stretch_t* stretch, const float* input, int inputSamples, double playbackRate);

void stretch_flush(stretch_t* stretch, float* output, int pcmOutLength, double playbackRate);

int stretch_seek_length(stretch_t* stretch);

void stretch_output_seek(stretch_t* stretch, const float* input, int inputLength);

int stretch_output_seek_length(stretch_t* stretch, float playbackRate);

void stretch_process(stretch_t* stretch, const float* input, int pcmLength, float* output, int pcmOutLength);

bool stretch_exact(stretch_t* stretch, const float* input, int pcmLength, float* output, int pcmOutLength);

#ifdef __cplusplus
}
#endif

#endif // SIGNALSMITH_STRETCH_WRAPPER_H
