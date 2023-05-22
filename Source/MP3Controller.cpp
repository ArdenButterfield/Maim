/*
  ==============================================================================

    MP3Controller.cpp
    Created: 18 Apr 2023 5:57:57pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "MP3Controller.h"

bool MP3Controller::init(const int sampleRate,
                         const int maxsampsperblock,
                         const int br)
{
    samplerate = validate_samplerate(sampleRate);
    bitrate = validate_bitrate(br);
    maxSamplesPerBlock = maxsampsperblock;
    
    input_buf_size = maxSamplesPerBlock;
    // From LAME api: mp3buf_size in bytes = 1.25*num_samples + 7200
    mp3_buf_size = input_buf_size * 1.25 + 7200;
    mp3Buffer.resize(mp3_buf_size);
    std::fill(mp3Buffer.begin(), mp3Buffer.end(), 0);

    outputBufferL = std::make_unique<QueueBuffer<float>>(1152 * 2 + maxSamplesPerBlock, 0.f);
    outputBufferR = std::make_unique<QueueBuffer<float>>(1152 * 2 + maxSamplesPerBlock, 0.f);

    if (!init_encoder()) {
        return false;
    }
    lame_dec_handler = hip_decode_init();
    bInitialized = true;
    return true;
}

void MP3Controller::deInit()
{
    bInitialized = false;
    deinit_encoder();
    
    if (lame_dec_handler) {
        hip_decode_exit(lame_dec_handler);
        lame_dec_handler = nullptr;
    }
    
    outputBufferL.reset(nullptr);
    outputBufferR.reset(nullptr);
    
    mp3Buffer.resize(0);
}

bool MP3Controller::copyOutput(float* left, float* right, const int num_block_samples)
{
    if (outputBufferL->num_items() < num_block_samples) {
        // std::cout << "Not enough items in queue.\n";
        return false;
    }
    if (left == nullptr) {
        for (int i = 0; i < num_block_samples; ++i) {
            outputBufferL->dequeue();
        }
    } else {
        for (int i = 0; i < num_block_samples; ++i) {
            left[i] = outputBufferL->dequeue();
        }
    }
    
    if (right == nullptr) {
        for (int i = 0; i < num_block_samples; ++i) {
            outputBufferR->dequeue();
        }
    } else {
        for (int i = 0; i < num_block_samples; ++i) {
            right[i] = outputBufferR->dequeue();
        }
    }
    return true;

}

int MP3Controller::samples_in_output_queue()
{
    return outputBufferL->num_items();
}
