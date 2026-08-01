#include <PluginProcessor.h>
#include <catch2/catch_test_macros.hpp>

#include "CodecControllers/OpusController.h"
#include "parameterIds.h"

#include <algorithm>
#include <cmath>
#include <vector>

// OpusController buffers one full 20ms frame of host samples before it can encode/decode it
// (see OpusController::processBlock), so that framing delay is the dominant, exactly-known
// component of its reported latency. These tests cover both the pure frame-size math and the
// end-to-end value MaimAudioProcessor reports to the host via getLatencySamples().

TEST_CASE ("opus latency is one 20ms frame at the nearest Opus-native rate", "[opuslatency]")
{
    // Exact matches to Opus's five native rates: 20ms of that exact rate.
    REQUIRE (OpusController::computeLatencySamples (8000.0) == 160);
    REQUIRE (OpusController::computeLatencySamples (12000.0) == 240);
    REQUIRE (OpusController::computeLatencySamples (16000.0) == 320);
    REQUIRE (OpusController::computeLatencySamples (24000.0) == 480);
    REQUIRE (OpusController::computeLatencySamples (48000.0) == 960);

    // 44.1kHz isn't an Opus-native rate, so it should snap to the nearest one (48000) before
    // computing the frame size - matching what OpusController::validate_samplerate() does.
    REQUIRE (OpusController::computeLatencySamples (44100.0) == 960);
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

TEST_CASE ("plugin still reports blade/lame's latency correctly", "[opuslatency]")
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

void processSilence (MaimAudioProcessor& plugin, int numSamples, int blockSize)
{
    int remaining = numSamples;
    while (remaining > 0) {
        int thisBlock = std::min (blockSize, remaining);
        juce::AudioBuffer<float> buffer (2, thisBlock);
        buffer.clear();
        auto midi = juce::MidiBuffer();
        plugin.processBlock (buffer, midi);
        remaining -= thisBlock;
    }
}

} // namespace

TEST_CASE ("measured latency roughly matches reported latency across encoders and samplerates", "[opuslatency]")
{
    auto gui = juce::ScopedJuceInitialiser_GUI {};

    constexpr int blockSize = 512;
    constexpr int flushSamples = 5000; // a few thousand samples to let an encoder switch settle
    constexpr int silenceLeadIn = 2000;
    constexpr int burstLength = 200;
    constexpr int silenceTrailOut = 5000;
    constexpr int totalSamples = silenceLeadIn + burstLength + silenceTrailOut;
    constexpr float burstFrequency = 1000.0f;
    constexpr float burstAmplitude = 0.8f;
    constexpr float onsetThreshold = 0.05f;

    const std::vector<double> sampleRates {44100.0, 48000.0, 96000.0};
    const std::vector<int> encoders {0, 1, 2}; // blade, lame, opus

    MaimAudioProcessor testPlugin;
    auto& apvts = testPlugin.getValueTreeState();

    for (double sampleRate : sampleRates) {
        testPlugin.prepareToPlay (sampleRate, blockSize);

        std::vector<float> inputSignal (static_cast<size_t> (totalSamples), 0.0f);
        for (int i = 0; i < burstLength; ++i) {
            inputSignal[static_cast<size_t> (silenceLeadIn + i)] =
                burstAmplitude * std::sin (2.0f * juce::MathConstants<float>::pi * burstFrequency
                    * static_cast<float> (i) / static_cast<float> (sampleRate));
        }

        for (int encoder : encoders) {
            // Switching to a new encoder (mid-stream, on the same live plugin instance) can
            // trigger an internal crossfade/settle period - flush it out before measuring.
            *((juce::AudioParameterChoice*) apvts.getParameter (ENCODER_PARAM_ID)) = encoder;
            processSilence (testPlugin, flushSamples, blockSize);

            auto reportedLatency = testPlugin.getLatencySamples();
            REQUIRE (reportedLatency > 0);

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

            // Measured latency = where the burst's onset actually shows up in the output, minus
            // where it started in the input.
            int measuredOnset = -1;
            for (int i = 0; i < totalSamples; ++i) {
                if (std::abs (outputSignal[static_cast<size_t> (i)]) > onsetThreshold) {
                    measuredOnset = i;
                    break;
                }
            }
            REQUIRE (measuredOnset >= 0);

            int measuredLatency = measuredOnset - silenceLeadIn;
            int tolerance = std::max (300, static_cast<int> (reportedLatency * 0.5));
            REQUIRE (std::abs (measuredLatency - reportedLatency) <= tolerance);
        }
    }
}
