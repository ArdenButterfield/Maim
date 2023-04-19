/*
  ==============================================================================

    MP3Controller.h
    Created: 18 Apr 2023 5:57:57pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once
#include <vector>
#include <array>
#include <memory>
#include <cstring>
#include <iostream>

#include <lame.h>

#include "QueueBuffer.h"

class MP3Controller
{
public:
    virtual ~MP3Controller() { deInit(); }
    bool init(const int sampleRate,
              const int maxSamplesPerBlock,
              const int bitrate);
    void deInit();
    virtual void addNextInput(float *left_input, float* right_input, const int num_block_samples) = 0;
    bool copyOutput(float* left, float* right, const int num_block_samples);
    int samples_in_output_queue();
protected:
//    void setButterflyBends(float buinbu, float buinbd, float bdinbu, float bdinbd);
//    void setMDCTbandstepBends(bool invert, int step);
//    void setMDCTpostshiftBends(int h_shift, float v_shift);
//    void setMDCTwindowincrBends(int window_incr);
//    void setMDCTBandReassignmentBends(int* order);
//    void setBitrateSquishBends(float squish);
//    void setThresholdBias(float bias);
//    void setMDCTfeedback(float feedback);
//
//    float* getPsychoanalThreshold();
//    float* getPsychoanalEnergy();
//    int getShortBlockStatus();
//
//    int getBitrate();
    float pcm_convert(short samp) {
        return samp / (float)std::numeric_limits<short>::max();
    }

    virtual bool init_encoder() = 0;
    virtual void deinit_encoder() = 0;
    virtual int validate_bitrate(int bitrate) = 0;
    virtual int validate_samplerate(int samplerate) = 0;
        
    bool bInitialized;
    hip_global_flags *lame_dec_handler = nullptr;
    
    std::vector<unsigned char> mp3Buffer;
    
    std::array<short, 20000> decodedLeftChannel = {0};
    std::array<short, 20000> decodedRightChannel = {0};
    
    std::vector<float> readBuf;
    int maxSamplesPerBlock;
    
    std::unique_ptr<QueueBuffer<float>> outputBufferL;
    std::unique_ptr<QueueBuffer<float>> outputBufferR;
    
    int bitrate;
    
    int input_buf_size;
    int mp3_buf_size;
};
