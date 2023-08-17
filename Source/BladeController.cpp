/*
  ==============================================================================

    BladeController.cpp
    Created: 18 Apr 2023 2:42:34pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "BladeController.h"

int BladeController::encodesamples(float* left, float* right)
{
    return blade_encode_chunk(blade_encoder, left, right, (char*)mp3Buffer.data());
}

bool BladeController::init_encoder()
{
    for (auto& buf: inputBuffer) {
        buf = std::make_unique<QueueBuffer<float>>(maxSamplesPerBlock + 2304, 0.f);
    }
    outputBufferL = std::make_unique<QueueBuffer<float>>(2304 + maxSamplesPerBlock, 0.f);
    outputBufferR = std::make_unique<QueueBuffer<float>>(2304 + maxSamplesPerBlock, 0.f);

    blade_encoder = blade_init(samplerate, bitrate);
    return true;
}

void BladeController::deinit_encoder()
{
    if (blade_encoder) {
        blade_deinit(blade_encoder);
    }
    blade_encoder = nullptr;
}

int BladeController::validate_samplerate(const int samplerate)
{
    return getClosest(samplerate, allowed_samplerates);
}

int BladeController::validate_bitrate(int bitrate)
{
    return getClosest(bitrate, allowed_bitrates);
}

int BladeController::getBitrate()
{
    return bitrate;
}

void BladeController::setButterflyBends(float buinbu, float buinbd, float bdinbu, float bdinbd)
{
    blade_set_butterfly_bends(blade_encoder, buinbu, buinbd, bdinbu, bdinbd);
}

void BladeController::setMDCTbandstepBends(bool invert, int step)
{
    
}

void BladeController::setMDCTpostshiftBends(int h_shift, float v_shift)
{
    float actual_v_shift;
    if (v_shift > 0) {
        actual_v_shift = v_shift / 100.f;
    } else {
        actual_v_shift = v_shift / 20.f;
    }

    blade_set_mdct_post_shift_bends(blade_encoder, h_shift, actual_v_shift);
}

void BladeController::setMDCTwindowincrBends(int window_incr)
{
    
}

void BladeController::setMDCTBandReassignmentBends(int* order)
{
    blade_set_mdct_band_reassignment_bends(blade_encoder, order);
}

void BladeController::setBitrateSquishBends(float squish)
{
    blade_set_bitrate_squish_bends(blade_encoder, (1 - squish) * (1 - squish) * (1 - squish));
}

void BladeController::setThresholdBias(float bias)
{
    blade_set_threshold_bias_bends(blade_encoder, bias);
}

void BladeController::setMDCTfeedback(float feedback)
{
    blade_set_mdct_feedback_bends(blade_encoder, feedback);
}

float* BladeController::getPsychoanalThreshold()
{
    return blade_get_psychoanal_threshold(blade_encoder);
}

float* BladeController::getPsychoanalEnergy()
{
    return blade_get_psychoanal_energy(blade_encoder);
}
float* BladeController::getMDCTpreBend() {
    return blade_get_mdct_pre_bend(blade_encoder);
}
float* BladeController::getMDCTpostBend() {
    return blade_get_mdct_post_bend(blade_encoder);
}


int BladeController::getShortBlockStatus()
{
    return blade_is_short_block(blade_encoder);
}

int BladeController::samplesInOutputQueue()
{
    return outputBufferL->num_items();
}
