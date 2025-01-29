//
// Created by arden on 8/27/24.
//

#ifndef MAIM_OPUSCONTROLLER_H
#define MAIM_OPUSCONTROLLER_H

#include "CodecController.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include "opus.h"
#include <array>
#include <vector>
#include "../QueueBuffer.h"

class OpusController : public CodecController {
public:
    OpusController()
        : bInitialized(false), opusEncoder(nullptr) {};
    ~OpusController() override = default;

    bool init(int sampleRate,
        int maxSamplesPerBlock,
        int bitrate
        ) override {
        deInit();
        auto error = 0;
        opusEncoder = opus_encoder_create(validate_samplerate(sampleRate), 2, OPUS_APPLICATION_VOIP, &error);

        return error == 0;
    }

    void deInit() override {
        if (opusEncoder != nullptr) {
            opus_encoder_destroy(opusEncoder);
            opusEncoder = nullptr;
        }
    }

    void processBlock(juce::AudioBuffer<float>& buffer) {

    }

    bool processFrame(float* leftIn, float* rightIn, float* leftOut, float* rightOut) override {

        // opus_encode_float(opusEncoder, interleavedInputFrames, encodedData)
    }

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

    bool bInitialized;
    OpusEncoder* opusEncoder;

    std::array<float, MP3FRAMESIZE * 2> interleavedInputFrames;
    std::array<unsigned char, 10000> encodedData;
    const std::vector<float> frameSizeOptionsMs {
        2.5, 5, 10, 20, 40, 60
    };

    const std::vector<int> allowed_samplerates {
        8000, 12000, 16000, 24000, 48000
    };

    std::vector<float> inputBuffer;
};

#endif //MAIM_OPUSCONTROLLER_H
