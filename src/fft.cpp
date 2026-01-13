#include <cstring>
#include "fft.h"
#include "signalsmith-stretch/fft.h"

struct fft_t {
    signalsmith::linear::FFT<float> fft;

    std::vector<std::complex<float>> inputBuffer;
    std::vector<std::complex<float>> outputBuffer;
};

fft_t* fft_create() {
    fft_t* fft = new fft_t();
    return fft;
}

void fft_delete(fft_t* fft) {
    delete fft;
}

void fft_resize(fft_t* fft, size_t size) {
    fft->fft.resize(size);
}

size_t fft_size(fft_t* fft) {
    return fft->fft.size();
}

size_t fft_steps(fft_t* fft) {
    return fft->fft.steps();
}

void fft_proc(fft_t* fft, const complex_t* input, complex_t* output) {
    size_t size = fft->fft.size();
    fft->inputBuffer.resize(size);
    fft->outputBuffer.resize(size);

    for (size_t i = 0; i < size; ++i) {
        fft->inputBuffer[i] = std::complex<float>(input[i].real, input[i].imag);
    }

    fft->fft.fft(fft->inputBuffer.data(), fft->outputBuffer.data());
    for (size_t i = 0; i < size; ++i) {
        output[i].real = fft->outputBuffer[i].real();
        output[i].imag = fft->outputBuffer[i].imag();
    }
}

void fft_proc_step(fft_t* fft, size_t step, const complex_t* input, complex_t* output) {
    size_t size = fft->fft.size();
    fft->inputBuffer.resize(size);
    fft->outputBuffer.resize(size);

    for (size_t i = 0; i < size; ++i) {
        fft->inputBuffer[i] = std::complex<float>(input[i].real, input[i].imag);
    }

    fft->fft.fft(step, fft->inputBuffer.data(), fft->outputBuffer.data());
    for (size_t i = 0; i < size; ++i) {
        output[i].real = fft->outputBuffer[i].real();
        output[i].imag = fft->outputBuffer[i].imag();
    }
}

void fft_proc_split(fft_t* fft, const float* inputReal, const float* inputImag, float* outputReal, float* outputImag) {
    fft->fft.fft(inputReal, inputImag, outputReal, outputImag);
}

void fft_proc_split_step(fft_t* fft, size_t step, const float* inputReal, const float* inputImag, float* outputReal, float* outputImag) {
    fft->fft.fft(step, inputReal, inputImag, outputReal, outputImag);
}

void fft_inverse_proc(fft_t* fft, const complex_t* input, complex_t* output) {
    size_t size = fft->fft.size();
    fft->inputBuffer.resize(size);
    fft->outputBuffer.resize(size);

    for (size_t i = 0; i < size; ++i) {
        fft->inputBuffer[i] = std::complex<float>(input[i].real, input[i].imag);
    }
    fft->fft.ifft(fft->inputBuffer.data(), fft->outputBuffer.data());
    for (size_t i = 0; i < size; ++i) {
        output[i].real = fft->outputBuffer[i].real();
        output[i].imag = fft->outputBuffer[i].imag();
    }
}

void fft_inverse_proc_step(fft_t* fft, size_t step, const complex_t* input, complex_t* output) {
    size_t size = fft->fft.size();
    fft->inputBuffer.resize(size);
    fft->outputBuffer.resize(size);

    for (size_t i = 0; i < size; ++i) {
        fft->inputBuffer[i] = std::complex<float>(input[i].real, input[i].imag);
    }

    fft->fft.ifft(step, fft->inputBuffer.data(), fft->outputBuffer.data());
    for (size_t i = 0; i < size; ++i) {
        output[i].real = fft->outputBuffer[i].real();
        output[i].imag = fft->outputBuffer[i].imag();
    }
}

void fft_inverse_proc_split(fft_t* fft, const float* inputReal, const float* inputImag, float* outputReal, float* outputImag) {
    fft->fft.ifft(inputReal, inputImag, outputReal, outputImag);
}

void fft_inverse_proc_split_step(fft_t* fft, size_t step, const float* inputReal, const float* inputImag, float* outputReal, float* outputImag) {
    fft->fft.ifft(step, inputReal, inputImag, outputReal, outputImag);
}