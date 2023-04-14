/*
  ==============================================================================

    LameController.cpp
    Created: 4 Apr 2023 8:59:01am
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "LameController.h"

float pcm_convert(short samp) {
    return samp / (float)std::numeric_limits<short>::max();
}

LameController::LameController(){
}

LameController::~LameController() { deInit(); }

bool LameController::init(const int sampleRate,
                          const int maxsampsperblock,
                          const int br) {
    bitrate = br;
    maxSamplesPerBlock = maxsampsperblock;
    
    input_buf_size = maxSamplesPerBlock;
    // From LAME api: mp3buf_size in bytes = 1.25*num_samples + 7200
    mp3_buf_size = input_buf_size * 1.25 + 7200;
    mp3Buffer.resize(mp3_buf_size);
    std::fill(mp3Buffer.begin(), mp3Buffer.end(), 0);
    
    outputBufferL = std::make_unique<QueueBuffer<float>>(1152 + maxSamplesPerBlock, 0.f);
    outputBufferR = std::make_unique<QueueBuffer<float>>(1152 + maxSamplesPerBlock, 0.f);
    
    
    lame_enc_handler = lame_init();
    lame_clear_bends(lame_enc_handler);
    if (!lame_enc_handler) {
        return false;
    }
    
    // Not all samplerates are supported by LAME.
    const int* test_rate = std::find(std::begin(allowed_samplerates),
                              std::end(allowed_samplerates),
                              sampleRate);
    int samp_rate_to_use;
    if (test_rate == std::end(allowed_samplerates)) {
        samp_rate_to_use = 44100;
    } else {
        samp_rate_to_use = sampleRate;
    }
    lame_set_in_samplerate(lame_enc_handler, samp_rate_to_use);
    lame_set_out_samplerate(lame_enc_handler, samp_rate_to_use);
    
    lame_set_brate(lame_enc_handler, bitrate);
    
    // Constant bitrate with no bit reservoir, to cut down on latency.
    // TODO: we get some sort of vector error when using the bir reservoir
    lame_set_VBR(lame_enc_handler, vbr_off);
    lame_set_disable_reservoir(lame_enc_handler, 1);
    if (lame_init_params(lame_enc_handler) != 0) {
        lame_close(lame_enc_handler);
        std::cout << "Bad params\n";
        return false;
    }

    lame_dec_handler = hip_decode_init();
    bInitialized = true;
    return true;
}

bool LameController::initialFlush()
{
    // The first encoding frame needs to be a bit larger than the subsequent
    // frames, and at low encoding delay, the start of the first frame can get
    // mangled, according to the LAME docs. To handle this, we flush the encoder
    // with a few frames of silence.
    const int initial_flush = 1152 * 3;

    float input_l[initial_flush] = {0};
    float input_r[initial_flush] = {0};

    int enc_result = lame_encode_buffer_ieee_float(
                lame_enc_handler,
                input_l,
                input_r,
                initial_flush,
                mp3Buffer.data(),
                mp3Buffer.size());
    
    if (enc_result < 0) {
        std::cout << enc_result<< " Unable to encode\n";
        return false;
    }
    
    int dec_result = hip_decode(lame_dec_handler,
                                mp3Buffer.data(),
                                enc_result,
                                decodedLeftChannel.data(),
                                decodedRightChannel.data());
    
    return true;
}

void LameController::deInit() {
    bInitialized = false;
    if (lame_enc_handler) {
        lame_close(lame_enc_handler);
        lame_enc_handler = nullptr;
    }
    if (lame_dec_handler) {
        hip_decode_exit(lame_dec_handler);
        lame_dec_handler = nullptr;
    }
    
    outputBufferL.reset(nullptr);
    outputBufferR.reset(nullptr);
    
    mp3Buffer.resize(0);
}

void LameController::addNextInput(float* left_input,
                                  float* right_input,
                                  const int num_block_samples) {
    if (!bInitialized) {
        std::cout << "Not initialized\n";
        return;
    }

    int enc_result = lame_encode_buffer_ieee_float(
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
    
    int dec_result = hip_decode(lame_dec_handler,
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
}

bool LameController::copyOutput(float* left, float* right, const int num_block_samples)
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

int LameController::samples_in_output_queue()
{
    return outputBufferL->num_items();
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
        actual_v_shift = v_shift / 2.f;
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

int LameController::getShortBlockStatus()
{
    return lame_is_short_block(lame_enc_handler);
}
