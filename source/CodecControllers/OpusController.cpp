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
      parameters(_parameters),
      encodeResult(0) {
    parameters.addParameterListener(BITRATE_PARAM_ID, this);
    parameters.addParameterListener(TURBO_PARAM_ID, this);

    parameters.addParameterListener(PACKET_LOSS_RATE_PARAM_ID, this);
    parameters.addParameterListener(PACKET_LOSS_PULSE_WIDTH_PARAM_ID, this);
    parameters.addParameterListener(PACKET_LOSS_BEAT_SYNC_PARAM_ID, this);
    parameters.addParameterListener(PACKET_LOSS_JITTER_PARAM_ID, this);
    parameters.addParameterListener(PACKET_LOSS_STICK_PARAM_ID, this);
    parameters.addParameterListener(PACKET_LOSS_MODE_PARAM_ID, this);
    parameters.addParameterListener(ERROR_PARAM_ID, this);
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
    parameters.removeParameterListener(PACKET_LOSS_STICK_PARAM_ID, this);
    parameters.removeParameterListener(PACKET_LOSS_MODE_PARAM_ID, this);
    parameters.removeParameterListener(ERROR_PARAM_ID, this);
}

bool OpusController::init (int sampleRate, int, int)
{
    deInit();
    auto initError = 0;
    auto validatedSamplerate = validate_samplerate(sampleRate);
    opusEncoder = opus_encoder_create(validatedSamplerate, 2, OPUS_APPLICATION_VOIP, &initError);
    if (initError) {
        return false;
    }
    initError = 0;
    opusDecoder = opus_decoder_create(validatedSamplerate, 2, &initError);
    if (initError) {
        return false;
    }
    samplesPerFrame = validatedSamplerate * 20 / 1000;

    input.resize(static_cast<size_t>(samplesPerFrame * 2));
    output.resize(static_cast<size_t>(samplesPerFrame * 2));

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
        input[static_cast<size_t>(sampleCounter * 2)] = buffer.getSample(0, s);
        input[static_cast<size_t>(sampleCounter * 2 + 1)] = buffer.getSample(1, s);
        buffer.setSample(0, s, output[static_cast<size_t>(sampleCounter * 2)]);
        buffer.setSample(1, s, output[static_cast<size_t>(sampleCounter * 2 + 1)]);
        output[static_cast<size_t>(sampleCounter * 2)] = 0;
        output[static_cast<size_t>(sampleCounter * 2 + 1)] = 0;
        sampleCounter++;
        if (sampleCounter == samplesPerFrame) {
            int framesizeDownscaleFactor = std::round(random.nextFloat() * turbo * 3);
            framesizeDownscaleFactor = 1 << std::min(std::max(0, framesizeDownscaleFactor), 2);
            auto subframeSize = samplesPerFrame / framesizeDownscaleFactor;
            for (int subframe = 0; subframe < framesizeDownscaleFactor; ++subframe) {
                float maxVolume = 0;
                for (auto samp = subframe * subframeSize; samp < subframe * subframeSize + subframeSize; ++samp) {
                    maxVolume = std::max(std::abs(input[static_cast<size_t>(samp)]), maxVolume);
                }
                auto encodeResultWithLoss = encodeResult;
                bool subframeLossOrStick = false;
                if (packetLossMode == PacketLossMode::on) {
                    subframeLossOrStick = true;
                } else if (packetLossMode == PacketLossMode::pulse && stickMode && !packetLossModel->processPacket(subframeSize)) {
                    subframeLossOrStick = true;
                }
                if (subframeLossOrStick) {
                    // do not encode.
                } else {

                    encodeResult = opus_encode_float(
                        opusEncoder, &input[0] + subframe * subframeSize,
                        subframeSize, &encoded[0], encoded.size());
                    if (error > 0) {
                        for (auto bit = 0; bit < encodeResult; ++bit) {
                            encoded[static_cast<size_t>(bit)] ^= random.nextFloat() < error ? 1 << random.nextInt(8) : 0;
                        }
                    }
                    subframeLossOrStick = packetLossMode == PacketLossMode::pulse && !packetLossModel->processPacket(subframeSize);
                    encodeResultWithLoss = subframeLossOrStick ? 0 : encodeResult;
                }
                [[maybe_unused]] auto decodeResult = opus_decode_float(
                    opusDecoder, &encoded[0], encodeResultWithLoss, &output[0] + subframe * subframeSize, subframeSize, 0);
                float maxOutputVolume = 0;
                for (auto samp = subframe * subframeSize; samp < subframe * subframeSize + subframeSize; ++samp) {
                    maxOutputVolume = std::max(std::abs(output[static_cast<size_t>(samp)]), maxOutputVolume);
                }
                auto outputThreshold = std::min(1.0f, maxVolume * 1.5f);
                if (maxOutputVolume > outputThreshold && maxOutputVolume > 0.0001) {
                    auto gain = outputThreshold / maxOutputVolume;
                    for (auto samp = subframe * subframeSize; samp < subframe * subframeSize + subframeSize; ++samp) {
                        output[static_cast<size_t>(samp)] *= gain;
                    }
                }
                // jassert(decodeResult == samplesPerFrame / framesizeDownscaleFactor);
            }
            sampleCounter = 0;
        }
    }
}

void OpusController::parameterChanged (const juce::String&, float)
{
    parametersNeedUpdating = true;
}

void OpusController::updateParameters()
{
    float packetLossLength = *((juce::AudioParameterFloat*)parameters.getParameter(PACKET_LOSS_RATE_PARAM_ID));
    float packetLossPulseWidth = *((juce::AudioParameterFloat*)parameters.getParameter(PACKET_LOSS_PULSE_WIDTH_PARAM_ID));
    float packetLossJitter = *((juce::AudioParameterFloat*)parameters.getParameter(PACKET_LOSS_JITTER_PARAM_ID));

    packetLossLength = (packetLossLength * packetLossLength * packetLossLength) * 2 + 0.05;

    packetLossModel->setParameters(packetLossLength, packetLossPulseWidth, packetLossJitter);

    turbo = parameters.getParameter(TURBO_PARAM_ID)->getValue();
    int newBitrate = Mp3ControllerManager::bitrates[static_cast<size_t>(((juce::AudioParameterChoice*)
                                parameters.getParameter(BITRATE_PARAM_ID))->getIndex())];
    opus_encoder_ctl(opusEncoder, OPUS_SET_BITRATE(newBitrate * 500));
    stickMode = *((juce::AudioParameterBool*)parameters.getParameter(PACKET_LOSS_STICK_PARAM_ID));
    packetLossMode = static_cast<PacketLossMode>(((juce::AudioParameterChoice*)parameters.getParameter(PACKET_LOSS_MODE_PARAM_ID))->getIndex());
    parametersNeedUpdating = false;

    error = *((juce::AudioParameterFloat*)parameters.getParameter(ERROR_PARAM_ID));
    error = std::pow(error, 3);
}
