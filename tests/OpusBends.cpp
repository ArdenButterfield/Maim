#include <PluginProcessor.h>
#include <catch2/catch_test_macros.hpp>
#include <juce_dsp/juce_dsp.h>

#include "CodecControllers/OpusController.h"
#include "parameterIds.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <functional>
#include <memory>
#include <vector>

// Tests for the Opus "bend" struct (OpusBendFlags), the equivalent of BendFlagsAndData (lame)
// and blade_bend_flags (blade) for the Opus encoder. Since libopus is used unmodified and has
// no exposed MDCT stage, the bends here operate on our own FFT of the PCM signal that flows
// into/out of the encoder, standing in for the frequency-domain hooks lame/blade have natively.

namespace {

using ParamSetter = std::function<void (juce::AudioProcessorValueTreeState&)>;

// Runs an OpusController through numFrames worth of 20ms frames at sampleRate (which must be
// one of OpusController's supported rates so no resampling/snapping occurs), feeding it a
// caller-provided signal and returning the decoded left-channel output.
std::vector<float> runOpusController (int sampleRate,
    int bufferSize,
    int numFrames,
    const ParamSetter& setParams,
    const std::function<float (int)>& inputGenerator)
{
    auto gui = juce::ScopedJuceInitialiser_GUI {};

    MaimAudioProcessor testPlugin;
    testPlugin.prepareToPlay (sampleRate, bufferSize);
    auto& apvts = testPlugin.getValueTreeState();

    if (setParams) {
        setParams (apvts);
    }

    OpusController opusController (apvts);
    REQUIRE (opusController.init (sampleRate, bufferSize, 100) == true);

    const int samplesPerFrame = sampleRate * 20 / 1000;
    const int totalSamples = samplesPerFrame * numFrames;

    std::vector<float> outputLeft;
    outputLeft.reserve (static_cast<size_t> (totalSamples));

    int sampleIndex = 0;
    while (sampleIndex < totalSamples) {
        int thisBlockSize = std::min (bufferSize, totalSamples - sampleIndex);
        juce::AudioBuffer<float> buffer (2, thisBlockSize);
        for (int s = 0; s < thisBlockSize; ++s) {
            float sample = inputGenerator (sampleIndex + s);
            buffer.setSample (0, s, sample);
            buffer.setSample (1, s, sample);
        }
        opusController.processBlock (buffer);
        for (int s = 0; s < thisBlockSize; ++s) {
            outputLeft.push_back (buffer.getSample (0, s));
        }
        sampleIndex += thisBlockSize;
    }
    return outputLeft;
}

std::function<float (int)> sineGenerator (float frequency, float amplitude, double sampleRate)
{
    return [frequency, amplitude, sampleRate] (int n) {
        return amplitude * std::sin (2.0f * juce::MathConstants<float>::pi * frequency * static_cast<float> (n) / static_cast<float> (sampleRate));
    };
}

bool allFinite (const std::vector<float>& v)
{
    return std::all_of (v.begin(), v.end(), [] (float x) { return std::isfinite (x); });
}

float peakAbs (const std::vector<float>& v, int fromEnd)
{
    auto start = v.size() >= static_cast<size_t> (fromEnd) ? v.end() - fromEnd : v.begin();
    float peak = 0.0f;
    for (auto it = start; it != v.end(); ++it) {
        peak = std::max (peak, std::abs (*it));
    }
    return peak;
}

// Computes the complex spectrum of the last fftSize samples of v (zero-padded if v is shorter).
std::vector<std::complex<float>> spectrumOfTail (const std::vector<float>& v, int fftOrder)
{
    const int fftSize = 1 << fftOrder;
    juce::dsp::FFT fft (fftOrder);
    std::vector<float> buffer (static_cast<size_t> (fftSize) * 2, 0.0f);
    size_t count = std::min (static_cast<size_t> (fftSize), v.size());
    std::copy (v.end() - static_cast<long> (count), v.end(), buffer.begin());
    fft.performRealOnlyForwardTransform (buffer.data(), true);

    auto* rawBins = reinterpret_cast<std::complex<float>*> (buffer.data());
    return std::vector<std::complex<float>> (rawBins, rawBins + (fftSize / 2 + 1));
}

float peakFrequency (const std::vector<std::complex<float>>& bins, double sampleRate, int fftSize)
{
    size_t peakBin = 1;
    float peakMag = 0.0f;
    for (size_t i = 1; i < bins.size(); ++i) {
        auto mag = std::abs (bins[i]);
        if (mag > peakMag) {
            peakMag = mag;
            peakBin = i;
        }
    }
    return static_cast<float> (static_cast<double> (peakBin) * sampleRate / fftSize);
}

float magnitudeNear (const std::vector<std::complex<float>>& bins, double sampleRate, int fftSize, float targetFreq, int tolBins)
{
    int targetBin = static_cast<int> (std::round (targetFreq * fftSize / sampleRate));
    float maxMag = 0.0f;
    for (int i = std::max (0, targetBin - tolBins); i <= std::min (static_cast<int> (bins.size()) - 1, targetBin + tolBins); ++i) {
        maxMag = std::max (maxMag, std::abs (bins[static_cast<size_t> (i)]));
    }
    return maxMag;
}

} // namespace

TEST_CASE ("opus band reassignment bend collapses reassigned bands to silence", "[opusbendreassignment]")
{
    constexpr int sampleRate = 48000;
    constexpr int bufferSize = 480;
    constexpr int numFrames = 8;
    // 5kHz sits well inside the destination bands (0-19) that BAND_ORDER_PARAM_IDS controls,
    // and well away from band 0 (roughly 0-730Hz), so redirecting every controllable band to
    // pull from band 0 should silence it.
    auto generator = sineGenerator (5000.0f, 0.6f, sampleRate);

    auto defaultOutput = runOpusController (sampleRate, bufferSize, numFrames, nullptr, generator);

    auto reassignToBandZero = [] (juce::AudioProcessorValueTreeState& apvts) {
        for (const auto& paramId : BAND_ORDER_PARAM_IDS) {
            *((juce::AudioParameterInt*) apvts.getParameter (paramId)) = 0;
        }
    };
    auto bentOutput = runOpusController (sampleRate, bufferSize, numFrames, reassignToBandZero, generator);

    REQUIRE (allFinite (defaultOutput));
    REQUIRE (allFinite (bentOutput));

    auto defaultPeak = peakAbs (defaultOutput, bufferSize);
    auto bentPeak = peakAbs (bentOutput, bufferSize);

    // The tone should come through clearly with no bend applied...
    REQUIRE (defaultPeak > 0.1f);
    // ...but collapse toward silence once every controllable band is redirected away from it.
    REQUIRE (bentPeak < defaultPeak * 0.3f);
}

TEST_CASE ("opus feedback bend at maximum forces silent output", "[opusbendfeedback]")
{
    constexpr int sampleRate = 48000;
    constexpr int bufferSize = 480;
    constexpr int numFrames = 8;
    auto generator = sineGenerator (1000.0f, 0.6f, sampleRate);

    auto defaultOutput = runOpusController (sampleRate, bufferSize, numFrames, nullptr, generator);

    auto maxFeedback = [] (juce::AudioProcessorValueTreeState& apvts) {
        *((juce::AudioParameterFloat*) apvts.getParameter (MDCT_FEEDBACK_PARAM_ID)) = 1.0f;
    };
    auto bentOutput = runOpusController (sampleRate, bufferSize, numFrames, maxFeedback, generator);

    REQUIRE (allFinite (defaultOutput));
    REQUIRE (allFinite (bentOutput));

    // At feedback == 1, every frame's spectrum is entirely replaced by the previous frame's,
    // and the previous frame starts at zero - so the signal never gets a chance to appear.
    REQUIRE (peakAbs (defaultOutput, bufferSize) > 0.1f);
    REQUIRE (peakAbs (bentOutput, bufferSize) < 1.0e-3f);
}

TEST_CASE ("opus horizontal spectral shift moves the dominant frequency higher", "[opusbendhshift]")
{
    constexpr int sampleRate = 48000;
    constexpr int bufferSize = 480;
    constexpr int numFrames = 10;
    constexpr int fftOrder = 10; // matches OpusController's internal FFT size at this sample rate
    constexpr int fftSize = 1 << fftOrder;
    auto generator = sineGenerator (500.0f, 0.6f, sampleRate);

    auto defaultOutput = runOpusController (sampleRate, bufferSize, numFrames, nullptr, generator);

    auto shiftUp = [] (juce::AudioProcessorValueTreeState& apvts) {
        *((juce::AudioParameterInt*) apvts.getParameter (MDCT_PITCH_SHIFT_PARAM_ID)) = 60;
    };
    auto bentOutput = runOpusController (sampleRate, bufferSize, numFrames, shiftUp, generator);

    REQUIRE (allFinite (defaultOutput));
    REQUIRE (allFinite (bentOutput));

    auto defaultPeakFreq = peakFrequency (spectrumOfTail (defaultOutput, fftOrder), sampleRate, fftSize);
    auto bentPeakFreq = peakFrequency (spectrumOfTail (bentOutput, fftOrder), sampleRate, fftSize);

    // 500Hz should survive un-shifted, and shifting by 60 bins (~2812Hz at this fft size) should
    // move the dominant frequency well above it.
    REQUIRE (defaultPeakFreq < 1000.0f);
    REQUIRE (bentPeakFreq > defaultPeakFreq + 1000.0f);
}

TEST_CASE ("opus vertical spectral shift attenuates quiet content relative to loud content", "[opusbendvshift]")
{
    constexpr int sampleRate = 48000;
    constexpr int bufferSize = 480;
    constexpr int numFrames = 10;
    constexpr int fftOrder = 10;
    constexpr int fftSize = 1 << fftOrder;
    constexpr float loudFreq = 300.0f;
    constexpr float quietFreq = 4000.0f;

    auto generator = [sampleRate] (int n) {
        return sineGenerator (loudFreq, 0.7f, sampleRate) (n) + sineGenerator (quietFreq, 0.03f, sampleRate) (n);
    };

    auto defaultOutput = runOpusController (sampleRate, bufferSize, numFrames, nullptr, generator);

    auto gateQuietContent = [] (juce::AudioProcessorValueTreeState& apvts) {
        *((juce::AudioParameterFloat*) apvts.getParameter (MDCT_AMPLITUDE_SHIFT_PARAM_ID)) = -1.0f;
    };
    auto bentOutput = runOpusController (sampleRate, bufferSize, numFrames, gateQuietContent, generator);

    REQUIRE (allFinite (defaultOutput));
    REQUIRE (allFinite (bentOutput));

    auto defaultSpectrum = spectrumOfTail (defaultOutput, fftOrder);
    auto bentSpectrum = spectrumOfTail (bentOutput, fftOrder);

    auto defaultLoud = magnitudeNear (defaultSpectrum, sampleRate, fftSize, loudFreq, 2);
    auto defaultQuiet = magnitudeNear (defaultSpectrum, sampleRate, fftSize, quietFreq, 2);
    auto bentLoud = magnitudeNear (bentSpectrum, sampleRate, fftSize, loudFreq, 2);
    auto bentQuiet = magnitudeNear (bentSpectrum, sampleRate, fftSize, quietFreq, 2);

    REQUIRE (defaultLoud > 0.0f);
    REQUIRE (bentLoud > 0.0f);

    // A negative vertical shift should shrink the quiet tone's contribution relative to the
    // loud tone's, compared to the unbent baseline.
    auto defaultRatio = defaultQuiet / defaultLoud;
    auto bentRatio = bentQuiet / bentLoud;
    REQUIRE (bentRatio < defaultRatio * 0.5f);
}

TEST_CASE ("opus bends at their default values behave exactly as before", "[opusbenddefault]")
{
    constexpr int sampleRate = 48000;
    constexpr int bufferSize = 480;
    constexpr int numFrames = 6;
    auto generator = sineGenerator (1000.0f, 0.6f, sampleRate);

    auto touchDefaults = [] (juce::AudioProcessorValueTreeState& apvts) {
        for (const auto& paramId : BAND_ORDER_PARAM_IDS) {
            auto* param = (juce::AudioParameterInt*) apvts.getParameter (paramId);
            *param = param->get(); // re-assert identity ordering, still index == value
        }
        *((juce::AudioParameterInt*) apvts.getParameter (MDCT_PITCH_SHIFT_PARAM_ID)) = 0;
        *((juce::AudioParameterFloat*) apvts.getParameter (MDCT_AMPLITUDE_SHIFT_PARAM_ID)) = 0.0f;
        *((juce::AudioParameterFloat*) apvts.getParameter (MDCT_FEEDBACK_PARAM_ID)) = 0.0f;
    };

    auto untouchedOutput = runOpusController (sampleRate, bufferSize, numFrames, nullptr, generator);
    auto touchedOutput = runOpusController (sampleRate, bufferSize, numFrames, touchDefaults, generator);

    REQUIRE (allFinite (untouchedOutput));
    REQUIRE (untouchedOutput.size() == touchedOutput.size());
    for (size_t i = 0; i < untouchedOutput.size(); ++i) {
        REQUIRE (untouchedOutput[i] == touchedOutput[i]);
    }
    REQUIRE (peakAbs (untouchedOutput, bufferSize) > 0.1f);
}

// Feeds numFrames of generator into a fresh OpusController and returns it (along with the
// owning plugin/apvts, which must outlive it) so the caller can inspect getMDCTpreBend()/
// getMDCTpostBend() - the data the live Spectral Effects graph is fed from.
struct OpusControllerRun {
    std::unique_ptr<juce::ScopedJuceInitialiser_GUI> gui;
    std::unique_ptr<MaimAudioProcessor> plugin;
    std::unique_ptr<OpusController> controller;
};

OpusControllerRun runOpusControllerKeepingItAlive (int sampleRate,
    int bufferSize,
    int numFrames,
    const ParamSetter& setParams,
    const std::function<float (int)>& inputGenerator)
{
    OpusControllerRun run;
    run.gui = std::make_unique<juce::ScopedJuceInitialiser_GUI>();
    run.plugin = std::make_unique<MaimAudioProcessor>();
    run.plugin->prepareToPlay (sampleRate, bufferSize);
    auto& apvts = run.plugin->getValueTreeState();

    if (setParams) {
        setParams (apvts);
    }

    run.controller = std::make_unique<OpusController> (apvts);
    REQUIRE (run.controller->init (sampleRate, bufferSize, 100) == true);

    const int samplesPerFrame = sampleRate * 20 / 1000;
    const int totalSamples = samplesPerFrame * numFrames;
    int sampleIndex = 0;
    while (sampleIndex < totalSamples) {
        int thisBlockSize = std::min (bufferSize, totalSamples - sampleIndex);
        juce::AudioBuffer<float> buffer (2, thisBlockSize);
        for (int s = 0; s < thisBlockSize; ++s) {
            float sample = inputGenerator (sampleIndex + s);
            buffer.setSample (0, s, sample);
            buffer.setSample (1, s, sample);
        }
        run.controller->processBlock (buffer);
        sampleIndex += thisBlockSize;
    }
    return run;
}

TEST_CASE ("opus exposes a live pre/post spectrum snapshot for the graph", "[opusbendspectrumgraph]")
{
    constexpr int sampleRate = 48000;
    constexpr int bufferSize = 480;
    constexpr int numFrames = 4;
    auto generator = sineGenerator (1000.0f, 0.6f, sampleRate);

    auto run = runOpusControllerKeepingItAlive (sampleRate, bufferSize, numFrames, nullptr, generator);

    auto* preBend = run.controller->getMDCTpreBend();
    auto* postBend = run.controller->getMDCTpostBend();
    REQUIRE (preBend != nullptr);
    REQUIRE (postBend != nullptr);

    bool preHasEnergy = false;
    for (int i = 0; i < 576; ++i) {
        REQUIRE (std::isfinite (preBend[i]));
        REQUIRE (std::isfinite (postBend[i]));
        // With no bend active, the "after" snapshot should match the "before" snapshot exactly.
        REQUIRE (preBend[i] == postBend[i]);
        if (preBend[i] > 0.001f) {
            preHasEnergy = true;
        }
    }
    REQUIRE (preHasEnergy);
}

TEST_CASE ("opus live spectrum graph reflects an active bend", "[opusbendspectrumgraph]")
{
    constexpr int sampleRate = 48000;
    constexpr int bufferSize = 480;
    constexpr int numFrames = 4;
    auto generator = sineGenerator (1000.0f, 0.6f, sampleRate);

    // feedback == 1 forces the post-bend spectrum toward silence (see the feedback bend test)
    // while the pre-bend spectrum still shows the incoming tone, so the two should diverge.
    auto maxFeedback = [] (juce::AudioProcessorValueTreeState& apvts) {
        *((juce::AudioParameterFloat*) apvts.getParameter (MDCT_FEEDBACK_PARAM_ID)) = 1.0f;
    };

    auto run = runOpusControllerKeepingItAlive (sampleRate, bufferSize, numFrames, maxFeedback, generator);

    auto* preBend = run.controller->getMDCTpreBend();
    auto* postBend = run.controller->getMDCTpostBend();

    bool preHasEnergy = false;
    bool postIsQuiet = true;
    for (int i = 0; i < 576; ++i) {
        REQUIRE (std::isfinite (preBend[i]));
        REQUIRE (std::isfinite (postBend[i]));
        if (preBend[i] > 0.001f) {
            preHasEnergy = true;
        }
        if (postBend[i] > 1.0e-3f) {
            postIsQuiet = false;
        }
    }
    REQUIRE (preHasEnergy);
    REQUIRE (postIsQuiet);
}
