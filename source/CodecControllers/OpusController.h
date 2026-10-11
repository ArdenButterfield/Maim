//
// Created by arden on 8/27/24.
//

#ifndef MAIM_OPUSCONTROLLER_H
#define MAIM_OPUSCONTROLLER_H

#include "CodecController.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_dsp/juce_dsp.h"
#include "opus.h"
#include <array>
#include <complex>
#include <vector>
#include "../QueueBuffer.h"
#include "PacketLossModel.h"
#include "OpusBendFlags.h"

class OpusController : public CodecController, public juce::AudioProcessorValueTreeState::Listener, public juce::Timer {
public:
    explicit OpusController(juce::AudioProcessorValueTreeState& _parameters);

    ~OpusController() override;

    bool init(int sampleRate,
        int maxSamplesPerBlock,
        int bitrate
        ) override;

    void deInit() override;

    void processBlock(juce::AudioBuffer<float>& buffer);

    int getBitrate() override { return bitrate; }

    // The number of host samples in one 20ms Opus frame, at whichever Opus-native rate
    // sampleRate snaps to (see validate_samplerate()).
    static int computeFrameSizeSamples(double sampleRate);

    // The number of host samples of delay Opus encoding/decoding introduces at sampleRate:
    // one full frame of buffering (see computeFrameSizeSamples()) plus the encoder's 6.5ms
    // lookahead. Usable without an initialized instance, so PluginProcessor can call it to
    // report latency to the host before/without an OpusController existing.
    static int computeLatencySamples(double sampleRate);

    void setError(float) override {}
    void setButterflyBends(float, float, float, float) override {}
    void setMDCTbandstepBends(bool, int) override {}
    void setMDCTwindowincrBends(int) override {}
    void setBitrateSquishBends(float) override {}
    void setThresholdBias(float) override {}

    void setMDCTpostshiftBends(int h_shift, float v_shift) override
    {
        bendFlags.mdctPostHShift = h_shift;
        bendFlags.mdctPostVShift = v_shift;
    }

    void setMDCTBandReassignmentBends(int* order) override
    {
        for (int i = 0; i < OpusBendFlags::numReassignmentBands; ++i) {
            bendFlags.mdctBandReassignment[static_cast<size_t>(i)] = order[i];
        }
    }

    void setMDCTfeedback(float feedback) override
    {
        bendFlags.mdctFeedback = feedback;
    }

    float* getPsychoanalThreshold() override {
        return nullptr;
    }
    float* getPsychoanalEnergy() override {
        return nullptr;
    }
    float* getMDCTpreBend() override {
        return mdctPreBend.data();
    }
    float* getMDCTpostBend() override {
        return mdctPostBend.data();
    }
    int getShortBlockStatus() override {
        return 0;
    }
protected:
    bool init_encoder() override {
        return true;
    }
    void deinit_encoder() override {}
    int validate_bitrate(int newBitrate) override {
        return newBitrate;
    }
    int validate_samplerate(int newSamplerate) override {
        return getClosest(newSamplerate,allowed_samplerates);
    }

    bool processFrame(float*, float*, float*, float*) override {

        // unused
        return false;
    }

    static constexpr int mdctDisplaySize = 576;

    void applyFrequencyBends();
    void applyBandReassignment(std::complex<float>* bins, int numBins);
    void applyPostShift(std::complex<float>* bins, int numBins);
    void applyFeedback(std::complex<float>* bins, int numBins, int channel);
    void accumulateSpectrumSnapshot(const std::complex<float>* bins, int numBins, std::array<float, mdctDisplaySize>& target) const;
    void timerCallback() override;

    bool bInitialized;
    OpusEncoder* opusEncoder;
    OpusDecoder* opusDecoder;

    std::array<float, MP3FRAMESIZE * 2> interleavedInputFrames{};
    std::array<unsigned char, 100000> encodedData{};
    const std::vector<float> frameSizeOptionsMs {
        2.5, 5, 10, 20, 40, 60
    };
    float turbo{};

    static inline const std::vector<int> allowed_samplerates {
        8000, 12000, 16000, 24000, 48000
    };

    std::vector<float> inputBuffer;
    int samplesPerFrame;
    std::vector<float> input{};
    std::vector<float> output{};
    std::array<unsigned char, 10000> encoded{};
    int sampleCounter;
    juce::AudioProcessorValueTreeState& parameters;
    bool parametersNeedUpdating;
    void parameterChanged(const juce::String &parameterID, float newValue) override;
    void updateParameters();
    juce::Random random;
    std::unique_ptr<PacketLossModel> packetLossModel;
    bool stickMode{};
    enum class PacketLossMode { off, pulse, on };
    PacketLossMode packetLossMode{PacketLossMode::pulse};
    int encodeResult{};
    float error{};

    OpusBendFlags bendFlags;
    std::unique_ptr<juce::dsp::FFT> fft;
    int fftOrder{};
    int fftSize{};
    std::array<std::vector<float>, 2> fftBuffers;
    std::vector<std::complex<float>> reassignmentScratch;
    std::array<std::vector<std::complex<float>>, 2> feedbackSpectrum;
    std::array<float, mdctDisplaySize> mdctPreBend{};
    std::array<float, mdctDisplaySize> mdctPostBend{};
};

#endif //MAIM_OPUSCONTROLLER_H
