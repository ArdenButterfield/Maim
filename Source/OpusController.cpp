//
// Created by arden on 8/27/24.
//

#include "OpusController.h"

bool OpusController::init (const int sampleRate, const int maxSamplesPerBlock, const int bitrate)
{
    return true;
}

bool OpusController::init_encoder()
{
    auto error = 0;
    opusEncoder = opus_encoder_create(samplerate, 2, OPUS_APPLICATION_AUDIO, &error);
    if (error) {
        return false;
    }
    return true;
}

void OpusController::deinit_encoder()
{
    if (opusEncoder) {
        opus_encoder_destroy(opusEncoder);
        opusEncoder = nullptr;
    }
}
int OpusController::validate_bitrate (int bitrate)
{
    return 0;
}
int OpusController::validate_samplerate (const int samplerate)
{
    return 0;
}
int OpusController::getBitrate()
{
    return bitrate;
}
void OpusController::setButterflyBends (float buinbu, float buinbd, float bdinbu, float bdinbd)
{
}
void OpusController::setMDCTbandstepBends (bool invert, int step)
{
}
void OpusController::setMDCTpostshiftBends (int h_shift, float v_shift)
{
}
void OpusController::setMDCTwindowincrBends (int window_incr)
{
}
void OpusController::setMDCTBandReassignmentBends (int* order)
{
}
void OpusController::setBitrateSquishBends (float squish)
{
}
void OpusController::_setThresholdBias (float bias)
{
}
void OpusController::setMDCTfeedback (float feedback)
{
}
float* OpusController::getPsychoanalThreshold()
{
    return nullptr;
}
float* OpusController::getPsychoanalEnergy()
{
    return nullptr;
}
float* OpusController::getMDCTpreBend()
{
    return nullptr;
}
float* OpusController::getMDCTpostBend()
{
    return nullptr;
}
int OpusController::getShortBlockStatus()
{
    return 0;
}