/*
  ==============================================================================

    BladeController.h
    Created: 18 Apr 2023 2:42:34pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once
#include "lib/blade/bladeenc/blade.h"

#include "MP3Controller.h"

class BladeController : public MP3Controller
{
public:
    BladeController() { bInitialized = false; }
    ~BladeController() { deInit(); }
    void addNextInput(float *left_input, float* right_input, const int num_block_samples) override;
    bool init_encoder() override;
    void deinit_encoder() override;
    int validate_bitrate(int bitrate) override;
    int validate_samplerate(const int samplerate) override;
    
    int samples_in_output_queue();
    
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
    float* getMDCTpreBend() override;
    float* getMDCTpostBend() override;

    int getShortBlockStatus() override;

private:
    encoder_flags_and_data* blade_encoder = nullptr;
    std::array<std::unique_ptr<QueueBuffer<float>>, 2> inputBuffer;
    
    // Values from the LAME documentation
    const std::vector<int> allowed_samplerates = {
        32000, 44100, 48000
    };

    const std::vector<int> allowed_bitrates = {
        32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320
    };
};
