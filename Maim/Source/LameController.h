/*
  ==============================================================================

    LameController.h
    Created: 4 Apr 2023 8:59:01am
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include "QueueBuffer.h"
#include "MP3Controller.h"

class LameController : public MP3Controller
{
public:
    LameController() { bInitialized = false; }
    ~LameController() { deInit(); }
    void addNextInput(float *left_input, float* right_input, const int num_block_samples) override;
    bool init_encoder() override;
    void deinit_encoder() override;
    int validate_bitrate(int bitrate) override;
    int validate_samplerate(const int samplerate) override;
    
    int getBitrate() override;
    void setButterflyBends(float buinbu, float buinbd, float bdinbu, float bdinbd) override;
    void setMDCTbandstepBends(bool invert, int step) override;
    void setMDCTpostshiftBends(int h_shift, float v_shift) override;
    void setMDCTwindowincrBends(int window_incr) override;
    void setMDCTBandReassignmentBends(int* order) override;
    void setBitrateSquishBends(float squish) override;
    void setThresholdBias(float bias) override;
    void setMDCTfeedback(float feedback) override;

    float* getPsychoanalThreshold() override;
    float* getPsychoanalEnergy() override;
    int getShortBlockStatus() override;

protected:
    lame_global_flags *lame_enc_handler = nullptr;

    // Values from the LAME documentation
    const std::array<int, 9> allowed_samplerates = {
        8000,
        11025,
        12000,
        16000,
        22050,
        24000,
        32000,
        44100,
        48000
    };
    
};
