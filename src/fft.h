#ifndef SIGNALSMITH_FFT_WRAPPER_H
#define SIGNALSMITH_FFT_WRAPPER_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fft_t fft_t;

typedef struct
{
    float real;
    float imag;
} complex_t;

fft_t* fft_create();

void fft_delete(fft_t* fft);

void fft_resize(fft_t* fft, size_t size);

size_t fft_size(fft_t* fft);

size_t fft_steps(fft_t* fft);

void fft_proc(fft_t* fft, const complex_t* input, complex_t* output);

void fft_proc_step(fft_t* fft, size_t step, const complex_t* input, complex_t* output);

void fft_proc_split(fft_t* fft, const float* inputReal, const float* inputImag, float* outputReal, float* outputImag);

void fft_proc_split_step(fft_t* fft, size_t step, const float* inputReal, const float* inputImag, float* outputReal, float* outputImag);

void fft_inverse_proc(fft_t* fft, const complex_t* input, complex_t* output);

void fft_inverse_proc_step(fft_t* fft, size_t step, const complex_t* input, complex_t* output);

void fft_inverse_proc_split(fft_t* fft, const float* inputReal, const float* inputImag, float* outputReal, float* outputImag);

void fft_inverse_proc_split_step(fft_t* fft, size_t step, const float* inputReal, const float* inputImag, float* outputReal, float* outputImag);

#ifdef __cplusplus
}
#endif

#endif // SIGNALSMITH_STFT_WRAPPER_H
