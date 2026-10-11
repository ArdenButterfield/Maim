#include <PluginProcessor.h>
#include <catch2/catch_test_macros.hpp>

#include "CodecControllers/OpusController.h"
#include "Mp3ControllerManager.h"
#include "parameterIds.h"

#include <algorithm>
#include <cmath>
#include <vector>

// OpusController buffers one full 20ms frame of host samples before it can encode/decode it
// (see OpusController::processBlock), and the Opus encoder adds 6.5ms of lookahead on top of
// that. These tests cover the pure frame-size/lookahead math, the end-to-end value
// MaimAudioProcessor reports to the host via getLatencySamples(), and how that compares to
// the delay actually measured through the plugin.

TEST_CASE ("opus latency is one 20ms frame plus 6.5ms lookahead at the nearest Opus-native rate", "[opuslatency]")
{
    // Exact matches to Opus's five native rates: 20ms + 6.5ms of that exact rate.
    REQUIRE (OpusController::computeLatencySamples (8000.0) == 160 + 52);
    REQUIRE (OpusController::computeLatencySamples (12000.0) == 240 + 78);
    REQUIRE (OpusController::computeLatencySamples (16000.0) == 320 + 104);
    REQUIRE (OpusController::computeLatencySamples (24000.0) == 480 + 156);
    REQUIRE (OpusController::computeLatencySamples (48000.0) == 960 + 312);

    // 44.1kHz isn't an Opus-native rate, so it should snap to the nearest one (48000) before
    // computing the frame size - matching what OpusController::validate_samplerate() does.
    REQUIRE (OpusController::computeLatencySamples (44100.0) == 960 + 312);
    REQUIRE (OpusController::computeFrameSizeSamples (44100.0) == 960);
}

TEST_CASE ("plugin reports opus's real latency instead of a hardcoded zero", "[opuslatency]")
{
    auto gui = juce::ScopedJuceInitialiser_GUI {};

    MaimAudioProcessor testPlugin;
    auto& apvts = testPlugin.getValueTreeState();
    *((juce::AudioParameterChoice*) apvts.getParameter (ENCODER_PARAM_ID)) = 2; // opus

    testPlugin.prepareToPlay (44100.0, 512);

    REQUIRE (testPlugin.getLatencySamples() == OpusController::computeLatencySamples (44100.0));
    REQUIRE (testPlugin.getLatencySamples() > 0);
}

TEST_CASE ("plugin still reports blade/lame's latency correctly", "[opuslatency][lamelatency][bladelatency]")
{
    // Regression test: currentLatencySamples() used to have an if/if-else chain where the
    // second branch's else clause (meant only for opus) unconditionally overwrote whatever
    // the first branch had just set, so blade always reported 0 too. Guards against that.
    auto gui = juce::ScopedJuceInitialiser_GUI {};

    MaimAudioProcessor bladePlugin;
    auto& bladeApvts = bladePlugin.getValueTreeState();
    *((juce::AudioParameterChoice*) bladeApvts.getParameter (ENCODER_PARAM_ID)) = 0; // blade
    bladePlugin.prepareToPlay (44100.0, 512);
    REQUIRE (bladePlugin.getLatencySamples() == 2209);

    MaimAudioProcessor lamePlugin;
    auto& lameApvts = lamePlugin.getValueTreeState();
    *((juce::AudioParameterChoice*) lameApvts.getParameter (ENCODER_PARAM_ID)) = 1; // lame
    lamePlugin.prepareToPlay (44100.0, 512);
    REQUIRE (lamePlugin.getLatencySamples() == 2880);
}

namespace {

// A hit with zero attack and a fairly sharp exponential decay: a high-frequency cosine, so
// its very first sample is also its peak. Preceded (and followed) by silence.
std::vector<float> makeHit (double sampleRate, int silenceLeadIn, int totalSamples, double frequency)
{
    constexpr float amplitude = 0.8f;
    constexpr double decaySeconds = 0.002; // time for the envelope to fall by a factor of e

    std::vector<float> signal (static_cast<size_t> (totalSamples), 0.0f);
    for (int i = silenceLeadIn; i < totalSamples; ++i) {
        double t = static_cast<double> (i - silenceLeadIn) / sampleRate;
        signal[static_cast<size_t> (i)] = amplitude
            * static_cast<float> (std::exp (-t / decaySeconds)
                * std::cos (2.0 * juce::MathConstants<double>::pi * frequency * t));
    }
    return signal;
}

int indexOfPeak (const std::vector<float>& signal)
{
    auto peak = std::max_element (signal.begin(), signal.end(),
        [] (float a, float b) { return std::abs (a) < std::abs (b); });
    return static_cast<int> (std::distance (signal.begin(), peak));
}

} // namespace

namespace {

struct HitLatencyMeasurement
{
    int reportedLatency;
    int measuredLatency;
    float outputPeakAmplitude;
};

// Runs a hit (see makeHit()) starting at silenceLeadIn through a fresh plugin instance, and
// measures latency as the distance between the hit's peak in the input and its peak in the
// output, rather than where the output first crosses a threshold - the latter fires early on
// a codec's pre-ringing, so it under-reads the delay of the hit itself.
HitLatencyMeasurement measureHitLatency (int encoder, int bitrateIndex, double sampleRate,
    int silenceLeadIn, int totalSamples, double frequency)
{
    constexpr int blockSize = 512;

    const auto inputSignal = makeHit (sampleRate, silenceLeadIn, totalSamples, frequency);
    REQUIRE (indexOfPeak (inputSignal) == silenceLeadIn);

    MaimAudioProcessor testPlugin;
    auto& apvts = testPlugin.getValueTreeState();
    *((juce::AudioParameterChoice*) apvts.getParameter (ENCODER_PARAM_ID)) = encoder;
    *((juce::AudioParameterChoice*) apvts.getParameter (BITRATE_PARAM_ID)) = bitrateIndex;
    testPlugin.prepareToPlay (sampleRate, blockSize);

    std::vector<float> outputSignal;
    outputSignal.reserve (static_cast<size_t> (totalSamples));

    int sampleIndex = 0;
    while (sampleIndex < totalSamples) {
        int thisBlock = std::min (blockSize, totalSamples - sampleIndex);
        juce::AudioBuffer<float> buffer (2, thisBlock);
        for (int s = 0; s < thisBlock; ++s) {
            float sample = inputSignal[static_cast<size_t> (sampleIndex + s)];
            buffer.setSample (0, s, sample);
            buffer.setSample (1, s, sample);
        }
        auto midi = juce::MidiBuffer();
        testPlugin.processBlock (buffer, midi);
        for (int s = 0; s < thisBlock; ++s) {
            outputSignal.push_back (buffer.getSample (0, s));
        }
        sampleIndex += thisBlock;
    }

    int outputPeak = indexOfPeak (outputSignal);
    return {
        testPlugin.getLatencySamples(),
        outputPeak - silenceLeadIn,
        std::abs (outputSignal[static_cast<size_t> (outputPeak)])
    };
}

} // namespace

TEST_CASE ("measured opus latency of a hit's peak matches reported latency across samplerates and bitrates", "[opuslatency]")
{
    auto gui = juce::ScopedJuceInitialiser_GUI {};

    constexpr double toleranceSeconds = 0.001;

    // Every Opus-native rate, plus host rates that have to snap to one.
    const std::vector<double> sampleRates {8000.0, 12000.0, 16000.0, 24000.0, 44100.0, 48000.0, 96000.0};
    // "32", "64", "128", "320". At "8" and "16" Opus smears the hit so badly that its peak
    // no longer says anything about where the hit is.
    const std::vector<int> bitrateIndices {3, 7, 11, 16};

    for (double sampleRate : sampleRates) {
        // Start the hit a quarter of the way into an Opus frame, well after the codec has
        // settled. OpusController scales each output frame down to 1.5x the peak of the input
        // frame it was encoded from, so a hit late in a frame gets delayed into the next
        // frame's output and squashed there, which distorts the peak we're trying to find.
        const int frameSize = OpusController::computeFrameSizeSamples (sampleRate);
        const int silenceLeadIn = frameSize * 12 + frameSize / 4;
        const int totalSamples = silenceLeadIn + frameSize * 12;
        const int tolerance = static_cast<int> (std::ceil (sampleRate * toleranceSeconds));

        for (int bitrateIndex : bitrateIndices) {
            auto [reportedLatency, measuredLatency, outputPeakAmplitude] = measureHitLatency (
                2, bitrateIndex, sampleRate, silenceLeadIn, totalSamples, sampleRate / 4.0);

            CAPTURE (sampleRate, Mp3ControllerManager::bitrates[static_cast<size_t> (bitrateIndex)],
                reportedLatency, measuredLatency, outputPeakAmplitude);
            // The hit has to actually survive the codec for its peak to mean anything.
            CHECK (outputPeakAmplitude > 0.2f);
            CHECK (std::abs (measuredLatency - reportedLatency) <= tolerance);
        }
    }
}

namespace {

// Blade and Lame report a fixed, experimentally-determined latency (see
// MaimAudioProcessor::currentLatencySamples()), so both get the same check.
void checkMp3HitLatency (int encoder)
{
    constexpr double toleranceSeconds = 0.001;
    constexpr int mp3FrameSize = 1152;

    // The three MPEG-1 rates, plus host rates above them.
    const std::vector<double> sampleRates {32000.0, 44100.0, 48000.0, 88200.0, 96000.0};
    // "64", "96", "128", "192", "320". Below that the hit's peak stops being a usable
    // landmark: Lame low-passes most of it away, and Blade can drop it altogether.
    const std::vector<int> bitrateIndices {7, 9, 11, 13, 16};

    for (double sampleRate : sampleRates) {
        // Well after the codec has settled, and not on a frame boundary.
        const int silenceLeadIn = mp3FrameSize * 20 + mp3FrameSize / 4;
        const int totalSamples = silenceLeadIn + mp3FrameSize * 12;
        const int tolerance = static_cast<int> (std::ceil (sampleRate * toleranceSeconds));

        for (int bitrateIndex : bitrateIndices) {
            auto [reportedLatency, measuredLatency, outputPeakAmplitude] = measureHitLatency (
                encoder, bitrateIndex, sampleRate, silenceLeadIn, totalSamples, sampleRate / 8.0);

            CAPTURE (sampleRate, Mp3ControllerManager::bitrates[static_cast<size_t> (bitrateIndex)],
                reportedLatency, measuredLatency, outputPeakAmplitude);
            // The hit has to actually survive the codec for its peak to mean anything.
            CHECK (outputPeakAmplitude > 0.2f);
            CHECK (std::abs (measuredLatency - reportedLatency) <= tolerance);
        }
    }
}

} // namespace

TEST_CASE ("measured lame latency of a hit's peak matches reported latency across samplerates and bitrates", "[lamelatency]")
{
    auto gui = juce::ScopedJuceInitialiser_GUI {};
    checkMp3HitLatency (1);
}

TEST_CASE ("measured blade latency of a hit's peak matches reported latency across samplerates and bitrates", "[bladelatency]")
{
    auto gui = juce::ScopedJuceInitialiser_GUI {};
    checkMp3HitLatency (0);
}
