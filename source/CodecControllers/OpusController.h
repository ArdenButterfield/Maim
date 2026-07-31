//
// Created by arden on 8/27/24.
//

#ifndef MAIM_OPUSCONTROLLER_H
#define MAIM_OPUSCONTROLLER_H

#include "CodecController.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_audio_processors/juce_audio_processors.h"
#include "opus.h"
#include <array>
#include <vector>
#include "../QueueBuffer.h"
#include "PacketLossModel.h"

class OpusController : public CodecController, public juce::AudioProcessorValueTreeState::Listener {
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
    void setError(float) override {}
    void setButterflyBends(float, float, float, float) override {}
    void setMDCTbandstepBends(bool, int) override {}
    void setMDCTpostshiftBends(int, float) override {}
    void setMDCTwindowincrBends(int) override {}
    void setMDCTBandReassignmentBends(int*) override {}
    void setBitrateSquishBends(float) override {}
    void setThresholdBias(float) override {}
    void setMDCTfeedback(float) override {}

    float* getPsychoanalThreshold() override {
        return nullptr;
    }
    float* getPsychoanalEnergy() override {
        return nullptr;
    }
    float* getMDCTpreBend() override {
        return nullptr;
    }
    float* getMDCTpostBend() override {
        return nullptr;
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


    bool bInitialized;
    OpusEncoder* opusEncoder;
    OpusDecoder* opusDecoder;

    std::array<float, MP3FRAMESIZE * 2> interleavedInputFrames{};
    std::array<unsigned char, 100000> encodedData{};
    const std::vector<float> frameSizeOptionsMs {
        2.5, 5, 10, 20, 40, 60
    };
    float turbo{};

    const std::vector<int> allowed_samplerates {
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
};

#endif //MAIM_OPUSCONTROLLER_H
