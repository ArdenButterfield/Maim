/*
  ==============================================================================

    MP3Controller.h
    Created: 18 Apr 2023 5:57:57pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#define WRITETODEBUGMP3FILE false

#include <limits>
#include <vector>
#include <array>
#include <memory>
#include <cstring>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "juce_core/juce_core.h"

#include "lame.h"

#include "../QueueBuffer.h"
#include "CodecController.h"

class MP3Controller : public CodecController
{
public:
    ~MP3Controller() override { }
    bool init(int sampleRate,
              int maxSamplesPerBlock,
              int bitrate
            ) override;
    void deInit() override;
    bool processFrame(float* leftIn, float* rightIn, float* leftOut, float* rightOut) override;


    std::string name;
    static const int MP3FRAMESIZE = 1152;
protected:
    void flushEncoder();

    static float pcmConvert (short samp) {
        return samp / (float)std::numeric_limits<short>::max();
    }
    float actualThresholdBias;

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

    int input_buf_size;
    int mp3_buf_size;

    static int getClosest(int target, const std::vector<int>& options);
#if WRITETODEBUGMP3FILE
    juce::File debugFile;
#endif

};
