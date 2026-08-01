//
// Created by arden on 8/27/24.
//

#include "OpusController.h"
#include "../parameterIds.h"
#include "../Mp3ControllerManager.h"
#include "../SpectrumRescale.h"
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
    parameters.addParameterListener(MDCT_PITCH_SHIFT_PARAM_ID, this);
    parameters.addParameterListener(MDCT_AMPLITUDE_SHIFT_PARAM_ID, this);
    parameters.addParameterListener(MDCT_FEEDBACK_PARAM_ID, this);
    for (const auto& bandOrderParamId : BAND_ORDER_PARAM_IDS) {
        parameters.addParameterListener(bandOrderParamId, this);
    }
    parametersNeedUpdating = true;
    startTimerHz(30);
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
    parameters.removeParameterListener(MDCT_PITCH_SHIFT_PARAM_ID, this);
    parameters.removeParameterListener(MDCT_AMPLITUDE_SHIFT_PARAM_ID, this);
    parameters.removeParameterListener(MDCT_FEEDBACK_PARAM_ID, this);
    for (const auto& bandOrderParamId : BAND_ORDER_PARAM_IDS) {
        parameters.removeParameterListener(bandOrderParamId, this);
    }
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

    fftOrder = 1;
    while ((1 << fftOrder) < samplesPerFrame) {
        ++fftOrder;
    }
    fftSize = 1 << fftOrder;
    fft = std::make_unique<juce::dsp::FFT>(fftOrder);

    const auto numBins = fftSize / 2 + 1;
    for (auto channel = 0; channel < 2; ++channel) {
        fftBuffers[static_cast<size_t>(channel)].assign(static_cast<size_t>(fftSize) * 2, 0.0f);
        feedbackSpectrum[static_cast<size_t>(channel)].assign(static_cast<size_t>(numBins), std::complex<float>(0.0f, 0.0f));
    }
    reassignmentScratch.assign(static_cast<size_t>(numBins), std::complex<float>(0.0f, 0.0f));

    return true;
}

int OpusController::computeLatencySamples(double sampleRate)
{
    auto validatedSamplerate = getClosest(static_cast<int>(std::lround(sampleRate)), allowed_samplerates);
    return validatedSamplerate * 20 / 1000;
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
            applyFrequencyBends();
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

    setMDCTpostshiftBends(
        ((juce::AudioParameterInt*) parameters.getParameter(MDCT_PITCH_SHIFT_PARAM_ID))->get(),
        ((juce::AudioParameterFloat*) parameters.getParameter(MDCT_AMPLITUDE_SHIFT_PARAM_ID))->get()
    );

    setMDCTfeedback(
        ((juce::AudioParameterFloat*) parameters.getParameter(MDCT_FEEDBACK_PARAM_ID))->get()
    );

    int bandReassign[OpusBendFlags::numReassignmentBands];
    int i;
    for (i = 0; i < NUM_REASSIGNMENT_BANDS; ++i) {
        bandReassign[i] = ((juce::AudioParameterInt*) parameters.getParameter(BAND_ORDER_PARAM_IDS[static_cast<size_t>(i)]))->get();
    }
    for (; i < OpusBendFlags::numReassignmentBands; ++i) {
        bandReassign[i] = i;
    }
    setMDCTBandReassignmentBends(bandReassign);
}

void OpusController::applyFrequencyBends()
{
    const auto numBins = fftSize / 2 + 1;
    const bool identity = bendFlags.isIdentity();

    // Accumulated across both channels, for the live "before/after" spectrum display - this
    // runs whether or not any bend is active, mirroring how lame/blade always snapshot their
    // MDCT spectrum for the graph regardless of bend settings.
    std::array<float, mdctDisplaySize> preSnapshot{};
    std::array<float, mdctDisplaySize> postSnapshot{};

    for (auto channel = 0; channel < 2; ++channel) {
        auto& fftBuffer = fftBuffers[static_cast<size_t>(channel)];
        std::fill(fftBuffer.begin(), fftBuffer.end(), 0.0f);
        for (auto s = 0; s < samplesPerFrame; ++s) {
            fftBuffer[static_cast<size_t>(s)] = input[static_cast<size_t>(s * 2 + channel)];
        }

        fft->performRealOnlyForwardTransform(fftBuffer.data(), true);

        auto* bins = reinterpret_cast<std::complex<float>*>(fftBuffer.data());

        accumulateSpectrumSnapshot(bins, numBins, preSnapshot);

        // When every bend is at its neutral value, skip mutating the spectrum and skip the
        // inverse transform entirely, so the encoded audio is bit-for-bit identical to before
        // this FFT stage existed - only the (otherwise unused) snapshot above is computed.
        if (!identity) {
            applyBandReassignment(bins, numBins);
            applyPostShift(bins, numBins);
            applyFeedback(bins, numBins, channel);
        }

        accumulateSpectrumSnapshot(bins, numBins, postSnapshot);

        if (!identity) {
            fft->performRealOnlyInverseTransform(fftBuffer.data());

            for (auto s = 0; s < samplesPerFrame; ++s) {
                input[static_cast<size_t>(s * 2 + channel)] = fftBuffer[static_cast<size_t>(s)];
            }
        }
    }

    for (auto i = 0; i < mdctDisplaySize; ++i) {
        mdctPreBend[static_cast<size_t>(i)] = preSnapshot[static_cast<size_t>(i)] / 2.0f;
        mdctPostBend[static_cast<size_t>(i)] = postSnapshot[static_cast<size_t>(i)] / 2.0f;
    }
}

void OpusController::accumulateSpectrumSnapshot(const std::complex<float>* bins, int numBins, std::array<float, mdctDisplaySize>& target) const
{
    // Normalizes FFT bin magnitude to roughly the 0-1(+) range rescaleMDCT() expects, and
    // resamples the (sample-rate-dependent) bin count down to the graph's fixed display width.
    const auto normalization = static_cast<float>(fftSize) / 2.0f;
    for (auto i = 0; i < mdctDisplaySize; ++i) {
        auto sourceBin = std::min(numBins - 1, (i * numBins) / mdctDisplaySize);
        target[static_cast<size_t>(i)] += std::abs(bins[static_cast<size_t>(sourceBin)]) / normalization;
    }
}

void OpusController::applyBandReassignment(std::complex<float>* bins, int numBins)
{
    const auto numBands = OpusBendFlags::numReassignmentBands;
    if (bendFlags.mdctBandReassignment == OpusBendFlags::identityReassignment()) {
        return;
    }

    std::copy(bins, bins + numBins, reassignmentScratch.begin());

    auto bandBoundary = [numBins, numBands] (int band) {
        return (band * numBins) / numBands;
    };

    for (auto band = 0; band < numBands; ++band) {
        auto destStart = bandBoundary(band);
        auto destEnd = bandBoundary(band + 1);
        auto srcBand = juce::jlimit(0, numBands - 1, bendFlags.mdctBandReassignment[static_cast<size_t>(band)]);
        auto srcStart = bandBoundary(srcBand);
        auto srcEnd = bandBoundary(srcBand + 1);
        auto count = std::min(destEnd - destStart, srcEnd - srcStart);
        for (auto i = 0; i < count; ++i) {
            bins[destStart + i] = reassignmentScratch[static_cast<size_t>(srcStart + i)];
        }
    }
}

void OpusController::applyPostShift(std::complex<float>* bins, int numBins)
{
    const auto hShift = bendFlags.mdctPostHShift;
    if (hShift != 0) {
        std::copy(bins, bins + numBins, reassignmentScratch.begin());
        for (auto i = 0; i < numBins; ++i) {
            auto srcIndex = i - hShift;
            bins[i] = (srcIndex >= 0 && srcIndex < numBins)
                ? reassignmentScratch[static_cast<size_t>(srcIndex)]
                : std::complex<float>(0.0f, 0.0f);
        }
    }

    const auto vShift = bendFlags.mdctPostVShift;
    if (vShift < 0.0f || vShift > 0.0f) {
        auto peak = 0.0f;
        for (auto i = 0; i < numBins; ++i) {
            peak = std::max(peak, std::abs(bins[i]));
        }
        if (peak > 0.0f) {
            for (auto i = 0; i < numBins; ++i) {
                auto mag = std::abs(bins[i]);
                float newMag;
                if (vShift > 0.0f) {
                    newMag = mag + vShift * (peak - mag);
                } else {
                    auto attenuation = std::max(0.0f, 1.0f + vShift * (1.0f - mag / peak));
                    newMag = mag * attenuation;
                }
                if (mag > 1.0e-8f) {
                    bins[i] *= (newMag / mag);
                } else if (vShift > 0.0f) {
                    bins[i] = std::complex<float>(newMag, 0.0f);
                }
            }
        }
    }
}

void OpusController::applyFeedback(std::complex<float>* bins, int numBins, int channel)
{
    auto& previous = feedbackSpectrum[static_cast<size_t>(channel)];
    const auto feedback = bendFlags.mdctFeedback;
    if (feedback > 0.0f) {
        for (auto i = 0; i < numBins; ++i) {
            bins[i] = bins[i] * (1.0f - feedback) + previous[static_cast<size_t>(i)] * feedback;
        }
    }
    std::copy(bins, bins + numBins, previous.begin());
}

void OpusController::timerCallback()
{
    // Mp3ControllerManager (lame/blade) owns this same "mdct" ValueTree node while it's the
    // active encoder - only push while Opus is actually selected, so the two don't fight.
    bool isOpus = (((juce::AudioParameterChoice*)parameters.getParameter(ENCODER_PARAM_ID))->getIndex() == 2);
    if (!isOpus) {
        return;
    }

    auto mdctSamples = parameters.state.getChildWithName("mdct");
    if (!mdctSamples.isValid()) {
        return;
    }

    juce::var preBendV, postBendV;
    for (auto i = 0; i < mdctDisplaySize; ++i) {
        preBendV.append(rescaleMDCT(mdctPreBend[static_cast<size_t>(i)]));
        postBendV.append(rescaleMDCT(mdctPostBend[static_cast<size_t>(i)]));
    }
    mdctSamples.setProperty("pre", preBendV, nullptr);
    mdctSamples.setProperty("post", postBendV, nullptr);
}
