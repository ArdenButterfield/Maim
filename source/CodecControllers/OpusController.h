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
        auto validatedSamplerate = validate_samplerate(sampleRate);
        opusEncoder = opus_encoder_create(validatedSamplerate, 2, OPUS_APPLICATION_VOIP, &error);
        if (error) {
            return false;
        }
        error = 0;
        opusDecoder = opus_decoder_create(validatedSamplerate, 2, &error);
        if (error) {
            return false;
        }
        samplesPerFrame = validatedSamplerate * 20 / 1000;

        input.resize(samplesPerFrame * 2);
        output.resize(samplesPerFrame * 2);

        sampleCounter = 0;

        return true;
    }

    void deInit() override {
        if (opusEncoder != nullptr) {
            opus_encoder_destroy(opusEncoder);
            opusEncoder = nullptr;
        }
    }

    void processBlock(juce::AudioBuffer<float>& buffer) {
        for (auto s = 0; s < buffer.getNumSamples(); ++s) {
            input[sampleCounter * 2] = buffer.getSample(0, s);
            input[sampleCounter * 2 + 1] = buffer.getSample(1, s);
            buffer.setSample(0, s, output[sampleCounter * 2]);
            buffer.setSample(1, s, output[sampleCounter * 2 + 1]);
            sampleCounter++;
            if (sampleCounter == samplesPerFrame) {
                auto encodeResult = opus_encode_float(opusEncoder, &input[0], samplesPerFrame, &encoded[0], encoded.size());
                jassert(encodeResult > 0);
                jassert(encodeResult < encoded.size());
                auto decodeResult = opus_decode_float(opusDecoder, &encoded[0], encodeResult, &output[0], samplesPerFrame, 0);
                jassert(decodeResult == samplesPerFrame);
                sampleCounter = 0;
            }
        }
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

    bool processFrame(float* leftIn, float* rightIn, float* leftOut, float* rightOut) override {

        // unused
        return false;
    }


    bool bInitialized;
    OpusEncoder* opusEncoder;
    OpusDecoder* opusDecoder;

    std::array<float, MP3FRAMESIZE * 2> interleavedInputFrames;
    std::array<unsigned char, 100000> encodedData;
    const std::vector<float> frameSizeOptionsMs {
        2.5, 5, 10, 20, 40, 60
    };

    const std::vector<int> allowed_samplerates {
        8000, 12000, 16000, 24000, 48000
    };

    std::vector<float> inputBuffer;
    int samplesPerFrame;
    std::vector<float> input;
    std::vector<float> output;
    std::array<unsigned char, 10000> encoded;
    int sampleCounter;
};

#endif //MAIM_OPUSCONTROLLER_H
