/*
  ==============================================================================

    MP3Controller.h
    Created: 18 Apr 2023 5:57:57pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <limits>
#include <vector>
#include <array>
#include <memory>
#include <cstring>
#include <iostream>
#include <algorithm>

#include <lame.h>

#include "QueueBuffer.h"


class MP3Controller
{
public:
    virtual ~MP3Controller() { }
    bool init(const int sampleRate,
              const int maxSamplesPerBlock,
              const int bitrate
            );
    void flushEncoder();
    void deInit();
    bool processFrame(float* leftIn, float* rightIn, float* leftOut, float* rightOut);

    virtual int getBitrate() = 0;
    virtual void setButterflyBends(float buinbu, float buinbd, float bdinbu, float bdinbd) = 0;
    virtual void setMDCTbandstepBends(bool invert, int step) = 0;
    virtual void setMDCTpostshiftBends(int h_shift, float v_shift) = 0;
    virtual void setMDCTwindowincrBends(int window_incr) = 0;
    virtual void setMDCTBandReassignmentBends(int* order) = 0;
    virtual void setBitrateSquishBends(float squish) = 0;
    virtual void setThresholdBias(float bias) = 0;
    virtual void setMDCTfeedback(float feedback) = 0;
    
    virtual float* getPsychoanalThreshold() = 0;
    virtual float* getPsychoanalEnergy() = 0;
    virtual float* getMDCTpreBend() = 0;
    virtual float* getMDCTpostBend() = 0;
    virtual int getShortBlockStatus() = 0;
    std::string name;
    static const int MP3FRAMESIZE = 1152;
protected:
    
    float pcmConvert (short samp) {
        return samp / (float)std::numeric_limits<short>::max();
    }

    virtual bool init_encoder() = 0;
    virtual void deinit_encoder() = 0;
    virtual int validate_bitrate(int bitrate) = 0;
    virtual int validate_samplerate(int samplerate) = 0;

    virtual int encodesamples(float* left, float* right) = 0;
    
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
    int samplerate;
    
    int input_buf_size;
    int mp3_buf_size;

    int getClosest(const int target, const std::vector<int>& options)
    {
        auto lower = options[0];
        auto upper = options[options.size() - 1];
        for (const auto option : options)
        {
            if (option < target)
            {
                lower = option;
            }
            else
            {
                upper = option;
                return (target - lower) < (upper - target) ? lower : upper;
            }
        }
        return lower;
    }
};
