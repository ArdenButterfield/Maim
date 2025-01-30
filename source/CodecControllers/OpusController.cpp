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
    parameters.addParameterListener(TURBO_PARAM_ID, this);

    parameters.addParameterListener(PACKET_LOSS_RATE_PARAM_ID, this);
    parameters.addParameterListener(PACKET_LOSS_PULSE_WIDTH_PARAM_ID, this);
    parameters.addParameterListener(PACKET_LOSS_BEAT_SYNC_PARAM_ID, this);
    parameters.addParameterListener(PACKET_LOSS_JITTER_PARAM_ID, this);

    parametersNeedUpdating = true;
}

OpusController::~OpusController()
{
    parameters.removeParameterListener(BITRATE_PARAM_ID, this);
    parameters.removeParameterListener(TURBO_PARAM_ID, this);

    parameters.removeParameterListener(PACKET_LOSS_RATE_PARAM_ID, this);
    parameters.removeParameterListener(PACKET_LOSS_PULSE_WIDTH_PARAM_ID, this);
    parameters.removeParameterListener(PACKET_LOSS_BEAT_SYNC_PARAM_ID, this);
    parameters.removeParameterListener(PACKET_LOSS_JITTER_PARAM_ID, this);
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

    packetLossModel = std::make_unique<PacketLossModel>(sampleRate);

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
            int framesizeDownscaleFactor = std::round(random.nextFloat() * turbo * 3);
            framesizeDownscaleFactor = 1 << std::min(std::max(0, framesizeDownscaleFactor), 2);
            auto subframeSize = samplesPerFrame / framesizeDownscaleFactor;
            for (int subframe = 0; subframe < framesizeDownscaleFactor; ++subframe) {
                auto encodeResult = opus_encode_float(
                    opusEncoder, &input[0] + subframe * subframeSize,
                    subframeSize, &encoded[0], encoded.size());
                jassert(encodeResult > 0);
                jassert(encodeResult < encoded.size());
                auto encodeResultWithLoss = packetLossModel->processPacket(subframeSize) ? encodeResult : 0;
                auto decodeResult = opus_decode_float(
                    opusDecoder, &encoded[0], encodeResultWithLoss, &output[0] + subframe * subframeSize, subframeSize, 0);
                jassert(decodeResult == samplesPerFrame / framesizeDownscaleFactor);
            }
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
    float packetLossLength = *((juce::AudioParameterFloat*)parameters.getParameter(PACKET_LOSS_RATE_PARAM_ID));
    float packetLossPulseWidth = *((juce::AudioParameterFloat*)parameters.getParameter(PACKET_LOSS_PULSE_WIDTH_PARAM_ID));
    float packetLossJitter = *((juce::AudioParameterFloat*)parameters.getParameter(PACKET_LOSS_JITTER_PARAM_ID));

    packetLossLength = (packetLossLength * packetLossLength * packetLossLength) * 2;

    packetLossModel->setParameters(packetLossLength, packetLossPulseWidth, packetLossJitter);

    turbo = parameters.getParameter(TURBO_PARAM_ID)->getValue();
    int bitrate = Mp3ControllerManager::bitrates[((juce::AudioParameterChoice*)
                                parameters.getParameter(BITRATE_PARAM_ID))->getIndex()];
    opus_encoder_ctl(opusEncoder, OPUS_SET_BITRATE(bitrate * 500));
    DBG("pack loss len " << packetLossLength << " ");
    parametersNeedUpdating = false;


}
