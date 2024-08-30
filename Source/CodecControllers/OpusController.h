//
// Created by arden on 8/27/24.
//

#ifndef MAIM_OPUSCONTROLLER_H
#define MAIM_OPUSCONTROLLER_H

#include "CodecController.h"
#include "opus.h"

class OpusController : public CodecController {
public:
    OpusController() : bInitialized(false) {};
    ~OpusController() override = default;

    bool init(int sampleRate,
        int maxSamplesPerBlock,
        int bitrate
        ) override {
        return true;
    }
    void deInit() override {}
    bool processFrame(float* leftIn, float* rightIn, float* leftOut, float* rightOut) override {
        return true;
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
        return samplerate;
    }

    bool bInitialized;
};

#endif //MAIM_OPUSCONTROLLER_H
