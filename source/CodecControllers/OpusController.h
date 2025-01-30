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
    explicit OpusController(juce::AudioProcessorValueTreeState& _parameters);;

    ~OpusController() override;

    bool init(int sampleRate,
        int maxSamplesPerBlock,
        int bitrate
        ) override;

    void deInit() override;

    void processBlock(juce::AudioBuffer<float>& buffer);

    int getBitrate() override { return bitrate; }
    void setButterflyBends(float buinbu, float buinbd, float bdinbu, float bdinbd) override {}
    void setMDCTbandstepBends(bool invert, int step) override {}
    void setMDCTpostshiftBends(int h_shift, float v_shift) override {}
    void setMDCTwindowincrBends(int window_incr) override {}
    void setMDCTBandReassignmentBends(int* order) override {}
    void setBitrateSquishBends(float squish) override {}
    void setThresholdBias(float bias) override {}
    void setMDCTfeedback(float feedback) override {}

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
    int validate_bitrate(int bitrate) override {
        return bitrate;
    }
    int validate_samplerate(int samplerate) override {
        return getClosest(samplerate,allowed_samplerates);
    }

    bool processFrame(float* leftIn, float* rightIn, float* leftOut, float* rightOut) override {

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
    float turbo;

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
};

#endif //MAIM_OPUSCONTROLLER_H
