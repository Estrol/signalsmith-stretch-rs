#ifndef SIGNALSMITH_STFT_WRAPPER_H
#define SIGNALSMITH_STFT_WRAPPER_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct stft_t stft_t;

enum stft_window_shape { ignore, acg, kaiser };

stft_t* stft_create(bool splitComputation);

void stft_delete(stft_t* stft);

void stft_configure(stft_t* stftBase, int inChannels, int outChannels, int blockSamples, int extraInputHistory, int intervalSamples, float asymmetry);

size_t stft_block_samples(stft_t* stftBase);

size_t stft_fft_samples(stft_t* stftBase);

size_t stft_bands(stft_t* stftBase);

void stft_reset(stft_t* stftBase);

void stft_write_input(stft_t* stftBase, size_t channel, size_t offset, size_t length, const float* inputArray);

void stft_read_output(stft_t* stftBase, size_t channel, size_t offset, size_t length, float* outputArray);

void stft_move_input(stft_t* stftBase, size_t samples, bool clearMovedRegion);

void stft_set_interval(stft_t* stftBase, size_t defaultInterval, enum stft_window_shape windowShape, float asymmetry);

void stft_analyse(stft_t* stftBase, size_t sampleInPast);

void stft_synthesise(stft_t* stftBase);

float* stft_spectrum(stft_t* stftBase, size_t channel);

float* stft_analysis_window(stft_t* stftBase);

float* stft_synthesis_window(stft_t* stftBase);

size_t stft_analysis_latency(stft_t* stftBase);

size_t stft_synthesis_latency(stft_t* stftBase);

size_t stft_latency(stft_t* stftBase);

float stft_bin_to_freq(stft_t* stftBase, float b);

float stft_freq_to_bin(stft_t* stftBase, float f);

size_t stft_analyse_steps(stft_t* stftBase);

size_t stft_synthesise_steps(stft_t* stftBase);

void stft_analyse_step(stft_t* stftBase, size_t step, size_t sampleInPast);

void stft_synthesise_step(stft_t* stftBase, size_t step);

size_t stft_samples_since_analysis(stft_t* stftBase);

size_t stft_samples_since_synthesis(stft_t* stftBase);

void stft_finish_output(stft_t* stftBase, float strength, size_t offset);

void stft_add_output(stft_t* stftBase, size_t channel, size_t offset, size_t length, const float* outputArray);

void stft_replace_output(stft_t* stftBase, size_t channel, size_t offset, size_t length, const float* outputArray);

void stft_move_output(stft_t* stftBase, size_t samples);

void stft_analysis_offset(stft_t* stftBase, size_t offset);

void stft_synthesis_offset(stft_t* stftBase, size_t offset);

#ifdef __cplusplus
}
#endif

#endif // SIGNALSMITH_STFT_WRAPPER_H
