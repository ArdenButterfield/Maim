/*
  ==============================================================================

    BladeController.cpp
    Created: 18 Apr 2023 2:42:34pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "BladeController.h"

void BladeController::addNextInput(float *left_input, float* right_input, const int num_block_samples)
{
    std::cout << "adding " << num_block_samples << " samples\n";
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
        std::cout << "encoding\n";
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
        std::cout << "got " << dec_result << " out\n";
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

bool BladeController::init_encoder()
{
    for (auto& buf: inputBuffer) {
        buf = std::make_unique<QueueBuffer<float>>(maxSamplesPerBlock + 2304, 0.f);
    }
    outputBufferL = std::make_unique<QueueBuffer<float>>(2304 + maxSamplesPerBlock, 0.f);
    outputBufferR = std::make_unique<QueueBuffer<float>>(2304 + maxSamplesPerBlock, 0.f);

    blade_encoder = blade_init(samplerate, bitrate);
}

void BladeController::deinit_encoder()
{
    if (blade_encoder) {
        blade_deinit(blade_encoder);
    }
    blade_encoder = nullptr;
}

int BladeController::validate_bitrate(int bitrate)
{
    auto test_rate = std::find(allowed_samplerates.begin(),
                              allowed_samplerates.end(),
                              samplerate);
    if (test_rate == allowed_samplerates.end()) {
        return 96;
    } else {
        return samplerate;
    }
}

int BladeController::validate_samplerate(const int samplerate)
{
    auto test_rate = std::find(allowed_samplerates.begin(),
                              allowed_samplerates.end(),
                              samplerate);
    if (test_rate == allowed_samplerates.end()) {
        return 44100;
    } else {
        return samplerate;
    }
}

int BladeController::getBitrate()
{
    return bitrate;
}

void BladeController::setButterflyBends(float buinbu, float buinbd, float bdinbu, float bdinbd)
{
    
}

void BladeController::setMDCTbandstepBends(bool invert, int step)
{
    
}

void BladeController::setMDCTpostshiftBends(int h_shift, float v_shift)
{
    
}

void BladeController::setMDCTwindowincrBends(int window_incr)
{
    
}

void BladeController::setMDCTBandReassignmentBends(int* order)
{
    
}

void BladeController::setBitrateSquishBends(float squish)
{
    
}

void BladeController::setThresholdBias(float bias)
{
    
}

void BladeController::setMDCTfeedback(float feedback)
{
    
}

float* BladeController::getPsychoanalThreshold()
{
    return nullptr;
}

float* BladeController::getPsychoanalEnergy()
{
    return nullptr;
}

int BladeController::getShortBlockStatus()
{
    return 0;
}

int BladeController::samples_in_output_queue()
{
    return outputBufferL->num_items();
}
