#include <cstring>
#include "stft.h"
#include "./signalsmith-stretch/stft.h"

// Abstract base class for STFT to allow for different template instantiations

class BaseSTFT {
public:
    float complex[2];

    virtual ~BaseSTFT() = default;

    virtual void configure(int inChannels, int outChannels, int blockSamples, int extraInputHistory, int intervalSamples, float asymmetry) = 0;
    virtual size_t blockSamples() const = 0;
    virtual size_t fftSamples() const = 0;
    virtual size_t defaultInterval() const = 0;
    virtual size_t bands() const = 0;
    virtual size_t analysisLatency() const = 0;
    virtual size_t synthesisLatency() const = 0;
    virtual size_t latency() const = 0;
    virtual void reset() = 0;

    virtual float binToFreq(float b) const = 0;
    virtual float freqToBin(float f) const = 0;

    virtual void writeInput(size_t channel, size_t offset, size_t length, const float* inputArray) = 0;
    virtual void moveInput(size_t samples, bool clearMovedRegion) = 0;

    virtual size_t samplesSinceAnalysis() const = 0;
    virtual void finishOutput(float strength = 0.0f, size_t offset = 0) = 0;
    virtual void readOutput(size_t channel, size_t offset, size_t length, float* outputArray) = 0;
    virtual void addOutput(size_t channel, size_t offset, size_t length, const float* outputArray) = 0;
    virtual void replaceOutput(size_t channel, size_t offset, size_t length, const float* outputArray) = 0;
    virtual void moveOutput(size_t samples) = 0;
    virtual size_t samplesSinceSynthesis() const = 0;
    virtual std::complex<float>* spectrum(size_t channel) = 0;
    virtual float* analysisWindow() = 0;
    virtual void analysisOffset(size_t offset) = 0;
    virtual float* synthesisWindow() = 0;
    virtual void synthesisOffset(size_t offset) = 0;

    virtual void setInterval(size_t defaultInterval, stft_window_shape windowShape, float asymmetry) = 0;
    virtual void analyse(size_t sampleInPast = 0) = 0;
    virtual size_t analyseSteps() const = 0;
    virtual void analyseStep(size_t step, size_t sampleInPast = 0) = 0;
    virtual void synthesise() = 0;
    virtual size_t synthesiseSteps() const = 0;
    virtual void synthesiseStep(size_t step) = 0;
};

class STFT : public BaseSTFT {
    signalsmith::linear::DynamicSTFT<float, false> stft;

    std::vector<std::complex<float>> inputBuffer;
    std::vector<std::complex<float>> outputBuffer;

public:
    ~STFT() override = default;

    void configure(int inChannels, int outChannels, int blockSamples, int extraInputHistory, int intervalSamples, float asymmetry) override {
        stft.configure(inChannels, outChannels, blockSamples, extraInputHistory, intervalSamples, asymmetry);
    }

    size_t blockSamples() const override {
        return stft.blockSamples();
    }

    size_t fftSamples() const override {
        return stft.fftSamples();
    }

    size_t defaultInterval() const override {
        return stft.defaultInterval();
    }

    size_t bands() const override {
        return stft.bands();
    }

    size_t analysisLatency() const override {
        return stft.analysisLatency();
    }

    size_t synthesisLatency() const override {
        return stft.synthesisLatency();
    }

    size_t latency() const override {
        return stft.latency();
    }

    void reset() override {
        stft.reset();
    }

    float binToFreq(float b) const override {
        return stft.binToFreq(b);
    }

    float freqToBin(float f) const override {
        return stft.freqToBin(f);
    }

    void writeInput(size_t channel, size_t offset, size_t length, const float* inputArray) override {
        stft.writeInput(channel, offset, length, inputArray);
    }

    void readOutput(size_t channel, size_t offset, size_t length, float* outputArray) override {
        stft.readOutput(channel, offset, length, outputArray);
    }

    void moveInput(size_t samples, bool clearMovedRegion) override {
        stft.moveInput(samples, clearMovedRegion);
    }

    size_t samplesSinceAnalysis() const override {
        return stft.samplesSinceAnalysis();
    }

    void finishOutput(float strength = 0.0f, size_t offset = 0) override {
        stft.finishOutput(strength, offset);
    }

    void addOutput(size_t channel, size_t offset, size_t length, const float* outputArray) override {
        stft.addOutput(channel, offset, length, outputArray);
    }

    void replaceOutput(size_t channel, size_t offset, size_t length, const float* outputArray) override {
        stft.replaceOutput(channel, offset, length, outputArray);
    }

    void moveOutput(size_t samples) override {
        stft.moveOutput(samples);
    }

    size_t samplesSinceSynthesis() const override {
        return stft.samplesSinceSynthesis();
    }

    std::complex<float>* spectrum(size_t channel) override {
        return stft.spectrum(channel);
    }

    float* analysisWindow() override {
        return stft.analysisWindow();
    }

    void analysisOffset(size_t offset) override {
        stft.analysisOffset(offset);
    }

    float* synthesisWindow() override {
        return stft.synthesisWindow();
    }

    void synthesisOffset(size_t offset) override {
        stft.synthesisOffset(offset);
    }

    void setInterval(size_t defaultInterval, stft_window_shape windowShape, float asymmetry) override {
        signalsmith::linear::DynamicSTFT<float, false>::WindowShape shape = signalsmith::linear::DynamicSTFT<float, false>::WindowShape::ignore;
        if (windowShape == acg) {
            shape = signalsmith::linear::DynamicSTFT<float, false>::acg;
        } else if (windowShape == kaiser) {
            shape = signalsmith::linear::DynamicSTFT<float, false>::kaiser;
        }
        stft.setInterval(defaultInterval, shape, asymmetry);
    }

    void analyse(size_t sampleInPast = 0) override {
        stft.analyse(sampleInPast);
    }

    size_t analyseSteps() const override {
        return stft.analyseSteps();
    }

    void analyseStep(size_t step, size_t sampleInPast = 0) override {
        stft.analyseStep(step, sampleInPast);
    }

    void synthesise() override {
        stft.synthesise();
    }

    size_t synthesiseSteps() const override {
        return stft.synthesiseSteps();
    }

    void synthesiseStep(size_t step) override {
        stft.synthesiseStep(step);
    }
};

class SFTF_SplitComputation : public BaseSTFT {
    signalsmith::linear::DynamicSTFT<float, true> stft;

    std::vector<std::complex<float>> inputBuffer;
    std::vector<std::complex<float>> outputBuffer;

public:
    ~SFTF_SplitComputation() override = default;

    void configure(int inChannels, int outChannels, int blockSamples, int extraInputHistory, int intervalSamples, float asymmetry) override {
        stft.configure(inChannels, outChannels, blockSamples, extraInputHistory, intervalSamples, asymmetry);
    }

    size_t blockSamples() const override {
        return stft.blockSamples();
    }

    size_t fftSamples() const override {
        return stft.fftSamples();
    }

    size_t defaultInterval() const override {
        return stft.defaultInterval();
    }

    size_t bands() const override {
        return stft.bands();
    }

    size_t analysisLatency() const override {
        return stft.analysisLatency();
    }

    size_t synthesisLatency() const override {
        return stft.synthesisLatency();
    }

    size_t latency() const override {
        return stft.latency();
    }

    void reset() override {
        stft.reset();
    }

    float binToFreq(float b) const override {
        return stft.binToFreq(b);
    }

    float freqToBin(float f) const override {
        return stft.freqToBin(f);
    }

    void writeInput(size_t channel, size_t offset, size_t length, const float* inputArray) override {
        stft.writeInput(channel, offset, length, inputArray);
    }

    void readOutput(size_t channel, size_t offset, size_t length, float* outputArray) override {
        stft.readOutput(channel, offset, length, outputArray);
    }

    void moveInput(size_t samples, bool clearMovedRegion) override {
        stft.moveInput(samples, clearMovedRegion);
    }

    size_t samplesSinceAnalysis() const override {
        return stft.samplesSinceAnalysis();
    }

    void finishOutput(float strength = 0.0f, size_t offset = 0) override {
        stft.finishOutput(strength, offset);
    }

    void addOutput(size_t channel, size_t offset, size_t length, const float* outputArray) override {
        stft.addOutput(channel, offset, length, outputArray);
    }

    void replaceOutput(size_t channel, size_t offset, size_t length, const float* outputArray) override {
        stft.replaceOutput(channel, offset, length, outputArray);
    }

    void moveOutput(size_t samples) override {
        stft.moveOutput(samples);
    }

    size_t samplesSinceSynthesis() const override {
        return stft.samplesSinceSynthesis();
    }

    std::complex<float>* spectrum(size_t channel) override {
        return stft.spectrum(channel);
    }

    float* analysisWindow() override {
        return stft.analysisWindow();
    }

    void analysisOffset(size_t offset) override {
        stft.analysisOffset(offset);
    }

    float* synthesisWindow() override {
        return stft.synthesisWindow();
    }

    void synthesisOffset(size_t offset) override {
        stft.synthesisOffset(offset);
    }

    void setInterval(size_t defaultInterval, stft_window_shape windowShape, float asymmetry) override {
        signalsmith::linear::DynamicSTFT<float, true>::WindowShape shape = signalsmith::linear::DynamicSTFT<float, true>::WindowShape::ignore;
        if (windowShape == acg) {
            shape = signalsmith::linear::DynamicSTFT<float, true>::acg;
        } else if (windowShape == kaiser) {
            shape = signalsmith::linear::DynamicSTFT<float, true>::kaiser;
        }
        stft.setInterval(defaultInterval, shape, asymmetry);
    }

    void analyse(size_t sampleInPast = 0) override {
        stft.analyse(sampleInPast);
    }

    size_t analyseSteps() const override {
        return stft.analyseSteps();
    }

    void analyseStep(size_t step, size_t sampleInPast = 0) override {
        stft.analyseStep(step, sampleInPast);
    }

    void synthesise() override {
        stft.synthesise();
    }

    size_t synthesiseSteps() const override {
        return stft.synthesiseSteps();
    }

    void synthesiseStep(size_t step) override {
        stft.synthesiseStep(step);
    }
};

struct stft_t {
    BaseSTFT* stft;
};

stft_t* stft_create(bool splitComputation) {
    stft_t* stftWrapper = new stft_t;
    if (splitComputation) {
        stftWrapper->stft = new SFTF_SplitComputation();
    } else {
        stftWrapper->stft = new STFT();
    }
    return stftWrapper;
}

void stft_delete(stft_t* stft) {
    delete stft->stft;
    delete stft;
}

void stft_configure(stft_t* stftBase, int inChannels, int outChannels, int blockSamples, int extraInputHistory, int intervalSamples, float asymmetry) {
    stftBase->stft->configure(inChannels, outChannels, blockSamples, extraInputHistory, intervalSamples, asymmetry);
}

size_t stft_block_samples(stft_t* stftBase) {
    return stftBase->stft->blockSamples();
}

size_t stft_fft_samples(stft_t* stftBase) {
    return stftBase->stft->fftSamples();
}

size_t stft_bands(stft_t* stftBase) {
    return stftBase->stft->bands();
}

void stft_reset(stft_t* stftBase) {
    stftBase->stft->reset();
}

void stft_write_input(stft_t* stftBase, size_t channel, size_t offset, size_t length, const float* inputArray) {
    stftBase->stft->writeInput(channel, offset, length, inputArray);
}

void stft_read_output(stft_t* stftBase, size_t channel, size_t offset, size_t length, float* outputArray) {
    stftBase->stft->readOutput(channel, offset, length, outputArray);
}

void stft_move_input(stft_t* stftBase, size_t samples, bool clearMovedRegion) {
    stftBase->stft->moveInput(samples, clearMovedRegion);
}

void stft_set_interval(stft_t* stftBase, size_t defaultInterval, stft_window_shape windowShape, float asymmetry) {
    stftBase->stft->setInterval(defaultInterval, windowShape, asymmetry);
}

void stft_analyse(stft_t* stftBase, size_t sampleInPast) {
    stftBase->stft->analyse(sampleInPast);
}

void stft_synthesise(stft_t* stftBase) {
    stftBase->stft->synthesise();
}

float* stft_spectrum(stft_t* stftBase, size_t channel) {
    std::complex<float>* spec = stftBase->stft->spectrum(channel);
    stftBase->stft->complex[0] = spec->real();
    stftBase->stft->complex[1] = spec->imag();

    return stftBase->stft->complex;
}

float* stft_analysis_window(stft_t* stftBase) {
    return stftBase->stft->analysisWindow();
}

float* stft_synthesis_window(stft_t* stftBase) {
    return stftBase->stft->synthesisWindow();
}

size_t stft_analysis_latency(stft_t* stftBase) {
    return stftBase->stft->analysisLatency();
}

size_t stft_synthesis_latency(stft_t* stftBase) {
    return stftBase->stft->synthesisLatency();
}

size_t stft_latency(stft_t* stftBase) {
    return stftBase->stft->latency();
}

float stft_bin_to_freq(stft_t* stftBase, float b) {
    return stftBase->stft->binToFreq(b);
}

float stft_freq_to_bin(stft_t* stftBase, float f) {
    return stftBase->stft->freqToBin(f);
}

size_t stft_analyse_steps(stft_t* stftBase) {
    return stftBase->stft->analyseSteps();
}

size_t stft_synthesise_steps(stft_t* stftBase) {
    return stftBase->stft->synthesiseSteps();
}

void stft_analyse_step(stft_t* stftBase, size_t step, size_t sampleInPast) {
    stftBase->stft->analyseStep(step, sampleInPast);
}

void stft_synthesise_step(stft_t* stftBase, size_t step) {
    stftBase->stft->synthesiseStep(step);
}

size_t stft_samples_since_analysis(stft_t* stftBase) {
    return stftBase->stft->samplesSinceAnalysis();
}

size_t stft_samples_since_synthesis(stft_t* stftBase) {
    return stftBase->stft->samplesSinceSynthesis();
}

void stft_finish_output(stft_t* stftBase, float strength, size_t offset) {
    stftBase->stft->finishOutput(strength, offset);
}

void stft_add_output(stft_t* stftBase, size_t channel, size_t offset, size_t length, const float* outputArray) {
    stftBase->stft->addOutput(channel, offset, length, outputArray);
}

void stft_replace_output(stft_t* stftBase, size_t channel, size_t offset, size_t length, const float* outputArray) {
    stftBase->stft->replaceOutput(channel, offset, length, outputArray);
}

void stft_move_output(stft_t* stftBase, size_t samples) {
    stftBase->stft->moveOutput(samples);
}

void stft_analysis_offset(stft_t* stftBase, size_t offset) {
    stftBase->stft->analysisOffset(offset);
}

void stft_synthesis_offset(stft_t* stftBase, size_t offset) {
    stftBase->stft->synthesisOffset(offset);
}