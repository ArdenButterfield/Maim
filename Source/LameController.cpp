/*
  ==============================================================================

    LameController.cpp
    Created: 4 Apr 2023 8:59:01am
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "LameController.h"

bool LameController::init_encoder()
{
    lame_enc_handler = lame_init();
    lame_clear_bends(lame_enc_handler);
    
    lame_set_in_samplerate(lame_enc_handler, samplerate);
    lame_set_out_samplerate(lame_enc_handler, samplerate);
    
    lame_set_brate(lame_enc_handler, bitrate);
    
    // Constant bitrate with no bit reservoir, to cut down on latency.
    lame_set_VBR(lame_enc_handler, vbr_off);
    lame_set_disable_reservoir(lame_enc_handler, 1);
    if (lame_init_params(lame_enc_handler) != 0) {
        lame_close(lame_enc_handler);
        std::cout << "Bad params\n";
        return false;
    }
    if (!lame_enc_handler) {
        return false;
    }
    return true;
}

void LameController::deinit_encoder()
{
    if (lame_enc_handler) {
        lame_close(lame_enc_handler);
        lame_enc_handler = nullptr;
    }
}

int LameController::validate_samplerate(const int samplerate)
{
    return getClosest(samplerate, allowed_samplerates);
}

int LameController::validate_bitrate(int bitrate)
{
    return getClosest(bitrate, allowed_bitrates);
}

void LameController::addNextInput(float* left_input,
                                  float* right_input,
                                  const int num_block_samples) {
    if (!bInitialized) {
        std::cout << "Not initialized\n";
        return;
    }

    const auto enc_result = lame_encode_buffer_ieee_float(
                lame_enc_handler,
                left_input,
                right_input,
                num_block_samples,
                mp3Buffer.data(),
                mp3Buffer.size());
    
    if (enc_result < 0) {
        std::cout << "Encoding error: " << enc_result << "\n";
        return;
    }
    
    const auto dec_result = hip_decode(lame_dec_handler,
                                mp3Buffer.data(),
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

    std::cout <<  name << "in: " << num_block_samples << "\tenc: " << enc_result << "\tdec: " << dec_result << "\toutbuf: " << samplesInOutputQueue() << "\n";
}

int LameController::getBitrate()
{
    return bitrate;
}

void LameController::setButterflyBends(float uu, float ud, float du, float dd)
{
    lame_set_butterfly_bends(lame_enc_handler, uu, ud, du, dd);
}

void LameController::setMDCTbandstepBends(bool invert, int step)
{
    lame_set_mdct_step_bends(lame_enc_handler, (int)invert, step);
}

void LameController::setMDCTfeedback(float feedback)
{
    lame_set_mdct_feedback_bends(lame_enc_handler, feedback);
}

void LameController::setMDCTpostshiftBends(int h_shift, float v_shift)
{
    float actual_v_shift;
    if (v_shift > 0) {
        actual_v_shift = v_shift / 100.f;
    } else {
        actual_v_shift = v_shift / 20.f;
    }
    lame_set_mdct_post_shift_bends(lame_enc_handler, h_shift, actual_v_shift);
}

void LameController::setMDCTwindowincrBends(int window_incr)
{
    lame_set_mdct_window_increment_bends(lame_enc_handler, window_incr);
}

void LameController::setMDCTBandReassignmentBends(int* order)
{
    lame_set_mdct_band_reassignment_bends(lame_enc_handler, order);
}

void LameController::setBitrateSquishBends(float squish)
{
    lame_set_bitrate_squish_bends(lame_enc_handler, (1 - squish) * (1 - squish) * (1 - squish));
}

void LameController::setThresholdBias(float bias)
{
    lame_set_threshold_bias_bends(lame_enc_handler, bias);
}

float* LameController::getPsychoanalThreshold()
{
    return lame_get_psychoanal_threshold(lame_enc_handler);
}

float* LameController::getPsychoanalEnergy()
{
    return lame_get_psychoanal_energy(lame_enc_handler);
}

float* LameController::getMDCTpreBend()
{
    return lame_get_mdct_pre_bend(lame_enc_handler);
}

float* LameController::getMDCTpostBend()
{
    return lame_get_mdct_post_bend(lame_enc_handler);
}

int LameController::getShortBlockStatus()
{
    return lame_is_short_block(lame_enc_handler);
}
