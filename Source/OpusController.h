//
// Created by arden on 8/27/24.
//

#ifndef MAIM_OPUSCONTROLLER_H
#define MAIM_OPUSCONTROLLER_H

#include "CodecController.h"
#include "opus.h"

class OpusController : public CodecController
{
public:
    OpusController() : opusEncoder(nullptr) { bInitialized = false; }
    ~OpusController() override { deInitOpus(); }
    bool init(int sampleRate, int maxSamplesPerBlock, int bitrate) override;
    void deInit() override { deInitOpus(); }
    bool processFrame(float *leftIn, float *rightIn, float *leftOut, float *rightOut) override { return false; }

    int getBitrate() override;
    void setButterflyBends(float buinbu, float buinbd, float bdinbu, float bdinbd) override;
    void setMDCTbandstepBends(bool invert, int step) override;
    void setMDCTpostshiftBends(int h_shift, float v_shift) override;
    void setMDCTwindowincrBends(int window_incr) override;
    void setMDCTBandReassignmentBends(int* order) override;
    void setBitrateSquishBends(float squish) override;
    void _setThresholdBias(float bias) override;
    void setMDCTfeedback(float feedback) override;

    float* getPsychoanalThreshold() override;
    float* getPsychoanalEnergy() override;
    float* getMDCTpreBend() override;
    float* getMDCTpostBend() override;

    int getShortBlockStatus() override;
private:
    bool init_encoder() override;
    void deinit_encoder() override;
    int validate_bitrate(int bitrate) override;
    int validate_samplerate(int samplerate) override;


    void deInitOpus() {}
    bool bInitialized;

    OpusEncoder* opusEncoder;

    const std::vector<int> allowed_samplerates = {
        32000, 44100, 48000
    };

    const std::vector<int> allowed_bitrates = {
        8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320
    };
    
};

#endif //MAIM_OPUSCONTROLLER_H
