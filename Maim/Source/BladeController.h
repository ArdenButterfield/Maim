/*
  ==============================================================================

    BladeController.h
    Created: 18 Apr 2023 2:42:34pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once
#include <blade.h>

#include "MP3Controller.h"

class BladeController : public MP3Controller
{
public:
    BladeController();
    ~BladeController();
    
    void init(const int sampleRate,
              const int maxSamplesPerBlock,
              const int bitrate);
    void deInit();
    void addNextInput(float *left_input, float* right_input, const int num_block_samples);
    bool copyOutput(float* left, float* right, const int num_block_samples);
    int samples_in_output_queue();

private:
    bool bInitialized = false;
    encoder_flags_and_data* blade_encoder = nullptr;
    hip_global_flags *lame_dec_handler = nullptr;
    
    std::array<std::unique_ptr<QueueBuffer<float>>, 2> inputBuffer;
    std::vector<char> mp3Buffer;
    
    std::array<short, 20000> decodedLeftChannel = {0};
    std::array<short, 20000> decodedRightChannel = {0};
    
    std::vector<float> readBuf;
    int maxSamplesPerBlock;
    std::unique_ptr<QueueBuffer<float>> outputBufferL;
    std::unique_ptr<QueueBuffer<float>> outputBufferR;
    
    int bitrate;
    
    int input_buf_size;
    int mp3_buf_size;
    
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
