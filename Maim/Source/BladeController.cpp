/*
  ==============================================================================

    BladeController.cpp
    Created: 18 Apr 2023 2:42:34pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "BladeController.h"

float pcm_convert(short samp) {
    return samp / (float)std::numeric_limits<short>::max();
}

BladeController::BladeController()
{
    bInitialized = false;
}

BladeController::~BladeController()
{
    deInit();
}

void BladeController::init(const int sampleRate,
          const int maxSamplesPerBlock,
          const int bitrate)
{
    input_buf_size = maxSamplesPerBlock;
    mp3_buf_size = input_buf_size * 1.25 + 7200;
    mp3Buffer.resize(mp3_buf_size);
    std::fill(mp3Buffer.begin(), mp3Buffer.end(), 0);

    for (auto& buf: inputBuffer) {
        buf = std::make_unique<QueueBuffer<float>>(maxSamplesPerBlock + 1152, 0.f);
    }
    outputBufferL = std::make_unique<QueueBuffer<float>>(1152 + maxSamplesPerBlock, 0.f);
    outputBufferR = std::make_unique<QueueBuffer<float>>(1152 + maxSamplesPerBlock, 0.f);
    
    lame_dec_handler = hip_decode_init();
    bInitialized = true;
    
    blade_encoder = blade_init(sampleRate, bitrate);
}

void BladeController::deInit()
{
    bInitialized = false;
    if (blade_encoder) {
        blade_deinit(blade_encoder);
        blade_encoder = nullptr;
    }
    if (lame_dec_handler) {
        hip_decode_exit(lame_dec_handler);
        lame_dec_handler = nullptr;
    }
    
    outputBufferL.reset(nullptr);
    outputBufferR.reset(nullptr);
    
    mp3Buffer.resize(0);

}

void BladeController::addNextInput(float *left_input, float* right_input, const int num_block_samples)
{
    for (int i = 0; i < num_block_samples; ++i) {
        inputBuffer[0]->enqueue(left_input[i]);
        inputBuffer[1]->enqueue(right_input[i]);
    }
    float left_chunk[1152];
    float right_chunk[1152];
    while (inputBuffer[0]->num_items() >= 1152) {
        for (int i = 0; i < 1152; ++i) {
            left_chunk[i] = inputBuffer[0]->dequeue();
            right_chunk[i] = inputBuffer[1]->dequeue();
        }
        int enc_result = blade_encode_chunk(blade_encoder,
                                            left_chunk,
                                            right_chunk,
                                            &mp3Buffer[0]);
        int dec_result = hip_decode(lame_dec_handler,
                                    (unsigned char*)&mp3Buffer[0],
                                    enc_result,
                                    decodedLeftChannel.data(),
                                    decodedRightChannel.data());
        
        if (dec_result < 0) {
            std::cout << "Decoding error: " << dec_result << "\n";
            return;
        }
        
        float amp;
        for (int i = 0; i < dec_result; ++i) {
            amp = pcm_convert(decodedLeftChannel[i]);
            outputBufferL->enqueue(amp);
        }
        for (int i = 0; i < dec_result; ++i) {
            amp = pcm_convert(decodedRightChannel[i]);
            outputBufferR->enqueue(amp);
        }
    }
}

int BladeController::samples_in_output_queue()
{
    return outputBufferL->num_items();
}

bool BladeController::copyOutput(float* left, float* right, const int num_block_samples)
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
