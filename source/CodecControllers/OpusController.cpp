//
// Created by arden on 8/27/24.
//

#include "OpusController.h"
#include "../parameterIds.h"
#include "../Mp3ControllerManager.h"
OpusController::OpusController (juce::AudioProcessorValueTreeState& _parameters)
    : bInitialized(false),
      opusEncoder(nullptr),
      opusDecoder(nullptr),
      samplesPerFrame(0),
      sampleCounter(0),
      parameters(_parameters) {
    parameters.addParameterListener(BITRATE_PARAM_ID, this);
    parametersNeedUpdating = true;
}

bool OpusController::init (int sampleRate, int maxSamplesPerBlock, int bitrate)
{
    deInit();
    auto error = 0;
    auto validatedSamplerate = validate_samplerate(sampleRate);
    opusEncoder = opus_encoder_create(validatedSamplerate, 2, OPUS_APPLICATION_VOIP, &error);
    if (error) {
        return false;
    }
    error = 0;
    opusDecoder = opus_decoder_create(validatedSamplerate, 2, &error);
    if (error) {
        return false;
    }
    samplesPerFrame = validatedSamplerate * 20 / 1000;

    input.resize(samplesPerFrame * 2);
    output.resize(samplesPerFrame * 2);

    sampleCounter = 0;
    parametersNeedUpdating = true;

    return true;
}

void OpusController::deInit()
{
    if (opusEncoder != nullptr) {
        opus_encoder_destroy(opusEncoder);
        opusEncoder = nullptr;
    }
}

void OpusController::processBlock (juce::AudioBuffer<float>& buffer)
{
    if (parametersNeedUpdating) {
        updateParameters();
    }
    for (auto s = 0; s < buffer.getNumSamples(); ++s) {
        input[sampleCounter * 2] = buffer.getSample(0, s);
        input[sampleCounter * 2 + 1] = buffer.getSample(1, s);
        buffer.setSample(0, s, output[sampleCounter * 2]);
        buffer.setSample(1, s, output[sampleCounter * 2 + 1]);
        sampleCounter++;
        if (sampleCounter == samplesPerFrame) {
            auto encodeResult = opus_encode_float(opusEncoder, &input[0], samplesPerFrame, &encoded[0], encoded.size());
            jassert(encodeResult > 0);
            jassert(encodeResult < encoded.size());
            auto decodeResult = opus_decode_float(opusDecoder, &encoded[0], encodeResult, &output[0], samplesPerFrame, 0);
            jassert(decodeResult == samplesPerFrame);
            sampleCounter = 0;
        }
    }
}

void OpusController::parameterChanged (const juce::String& parameterID, float newValue)
{
    parametersNeedUpdating = true;
}

void OpusController::updateParameters()
{
    int bitrate = Mp3ControllerManager::bitrates[((juce::AudioParameterChoice*)
                                parameters.getParameter(BITRATE_PARAM_ID))->getIndex()];
    opus_encoder_ctl(opusEncoder, OPUS_SET_BITRATE(bitrate * 1000));
    DBG("Opus bitrate set to " << bitrate << " ");
    parametersNeedUpdating = false;
}
