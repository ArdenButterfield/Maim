
#include "helpers/test_helpers.h"

#include <PluginProcessor.h>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "BinaryData.h"

TEST_CASE ("one is equal to one", "[dummy]")
{
    REQUIRE (1 == 1);
}

TEST_CASE ("Plugin instance", "[instance]")
{

    // This lets us use JUCE's MessageManager without leaking.
    // PluginProcessor might need this if you use the APVTS for example.
    // You'll also need it for tests that rely on juce::Graphics, juce::Timer, etc.
    auto gui = juce::ScopedJuceInitialiser_GUI {};

    MaimAudioProcessor testPlugin;


    SECTION ("name")
    {
        CHECK_THAT (testPlugin.getName().toStdString(),
            Catch::Matchers::Equals ("Maim"));
    }
}

TEST_CASE("process audio", "[process]")
{
    // This lets us use JUCE's MessageManager without leaking.
    // PluginProcessor might need this if you use the APVTS for example.
    // You'll also need it for tests that rely on juce::Graphics, juce::Timer, etc.
    auto gui = juce::ScopedJuceInitialiser_GUI {};

    MaimAudioProcessor testPlugin;
    testPlugin.prepareToPlay(44100, 512);
    auto& apvts = testPlugin.getValueTreeState();
    for (int i = 0; i < 3; ++i) {
        *((juce::AudioParameterChoice*)apvts.getParameter("encoder")) = i;

        juce::AudioBuffer<float> samples(2,512);
        for (int i = 0; i < 10; ++i) {
            for (int samp = 0; samp < 512; ++samp) {
                samples.setSample(0, i, sin(static_cast<float>(samp) / 100.f));
                samples.setSample(1, i, sin(static_cast<float>(samp) / 100.f));
            }
            auto midiBuffer = juce::MidiBuffer();
            testPlugin.processBlock(samples, midiBuffer);
        }
        REQUIRE(samples.findMinMax(0,0,samples.getNumSamples()).getEnd() > 0.1f);
    }
}




juce::AudioBuffer<float> makeTestAudioBuffer() {
    auto inputStream = new juce::MemoryInputStream(BinaryData::kyoto10sec_wav, BinaryData::kyoto10sec_wavSize, false);
    juce::WavAudioFormat wavFormat;
    std::unique_ptr<juce::AudioFormatReader> reader(wavFormat.createReaderFor(inputStream, false));
    juce::AudioBuffer<float> buffer(reader->numChannels, reader->lengthInSamples);
    reader->read(&buffer, 0, buffer.getNumSamples(), 0, true, true);
    return buffer;
}

void writeOut(const juce::AudioBuffer<float>& buffer, const juce::String& name) {
    juce::WavAudioFormat format;
    std::unique_ptr<juce::AudioFormatWriter> writer;
    writer.reset (format.createWriterFor (
        new juce::FileOutputStream (
            juce::File(juce::File::getCurrentWorkingDirectory().getChildFile(name))),
        44100.0,
        buffer.getNumChannels(),
        24,
        {},
        0));
    if (writer != nullptr)
        writer->writeFromAudioSampleBuffer (buffer, 0, buffer.getNumSamples());
}

TEST_CASE("base opus", "[baseopus]")
{
    auto input = makeTestAudioBuffer();
    writeOut(input, "original.wav");
    auto error = 0;
    auto opus_encoder = opus_encoder_create(48000, 2, OPUS_APPLICATION_VOIP, &error);
    REQUIRE(error == 0);
    auto opus_decoder = opus_decoder_create(48000, 2, &error);
    REQUIRE(error == 0);
    int samples_per_frame = 10 /* ms / frame */ * 48 /* samples / ms */;
    REQUIRE(input.getNumChannels() == 2);
    auto buf = juce::AudioBuffer<float>(input.getNumChannels(), samples_per_frame);
    auto output = juce::AudioBuffer<float>(input);
    output.clear();
    opus_encoder_ctl(opus_encoder, OPUS_SET_BITRATE(50000));
    std::vector<float> interlacedInput;
    std::vector<float> interlacedOutput;
    interlacedInput.resize(samples_per_frame * 2);
    interlacedOutput.resize(samples_per_frame * 2);
    std::array<unsigned char, 10000> encoded{};
    for (int i = 0; i < input.getNumSamples() - samples_per_frame; i += samples_per_frame) {
        for (int samp = 0; samp < samples_per_frame; ++samp) {
            interlacedInput[samp * 2] = input.getSample(0, i + samp);
            interlacedInput[samp * 2 + 1] = input.getSample(1, i + samp);
        }
        auto encodeResult = opus_encode_float(opus_encoder, &interlacedInput[0], samples_per_frame, &encoded[0], 10000);
        REQUIRE(encodeResult > 0);
        auto decodeResult = opus_decode_float(opus_decoder, &encoded[0], encodeResult, &interlacedOutput[0], samples_per_frame, 0);
        REQUIRE(decodeResult == samples_per_frame);
        for (int samp = 0; samp < samples_per_frame; ++samp) {
            output.setSample(0, samp + i, interlacedOutput[samp * 2]);
            output.setSample(1, samp + i, interlacedOutput[samp * 2 + 1]);
        }
    }
    writeOut(output, "50k_opus.wav");
}

TEST_CASE("framedrop opus", "[framedropopus]")
{
    auto input = makeTestAudioBuffer();
    writeOut(input, "original.wav");
    auto error = 0;
    auto opus_encoder = opus_encoder_create(48000, 2, OPUS_APPLICATION_VOIP, &error);
    REQUIRE(error == 0);
    auto opus_decoder = opus_decoder_create(48000, 2, &error);
    REQUIRE(error == 0);
    int samples_per_frame = 10 /* ms / frame */ * 48 /* samples / ms */;
    REQUIRE(input.getNumChannels() == 2);
    auto buf = juce::AudioBuffer<float>(input.getNumChannels(), samples_per_frame);
    auto output = juce::AudioBuffer<float>(input);
    output.clear();
    opus_encoder_ctl(opus_encoder, OPUS_SET_BITRATE(50000));
    std::vector<float> interlacedInput;
    std::vector<float> interlacedOutput;
    interlacedInput.resize(samples_per_frame * 2);
    interlacedOutput.resize(samples_per_frame * 2);
    std::array<unsigned char, 10000> encoded{};
    int framedrop_counter = 0;
    for (int i = 0; i < input.getNumSamples() - samples_per_frame; i += samples_per_frame) {
        for (int samp = 0; samp < samples_per_frame; ++samp) {
            interlacedInput[samp * 2] = input.getSample(0, i + samp);
            interlacedInput[samp * 2 + 1] = input.getSample(0, i + samp + 1);
        }
        auto encodeResult = 0;
        if (framedrop_counter < 1) {
            encodeResult = opus_encode_float(opus_encoder, &interlacedInput[0], samples_per_frame, &encoded[0], 10000);
            REQUIRE(encodeResult > 0);
        }
        opus_decode_float(opus_decoder, &encoded[0], encodeResult, &interlacedOutput[0], samples_per_frame, 0);
        for (int samp = 0; samp < samples_per_frame; ++samp) {
            output.setSample(0, samp + i, interlacedOutput[samp * 2]);
            output.setSample(1, samp + i + 1, interlacedOutput[samp * 2 + 1]);
        }
        framedrop_counter ++;
        framedrop_counter %= 3;
    }
    writeOut(output, "50k_framedrop_opus.wav");
}

TEST_CASE("corrupt opus", "[corruptopus]")
{
    auto input = makeTestAudioBuffer();
    writeOut(input, "original.wav");
    auto error = 0;
    auto opus_encoder = opus_encoder_create(48000, 2, OPUS_APPLICATION_VOIP, &error);
    REQUIRE(error == 0);
    auto opus_decoder = opus_decoder_create(48000, 2, &error);
    REQUIRE(error == 0);
    int samples_per_frame = 10 /* ms / frame */ * 48 /* samples / ms */;
    REQUIRE(input.getNumChannels() == 2);
    auto buf = juce::AudioBuffer<float>(input.getNumChannels(), samples_per_frame);
    auto output = juce::AudioBuffer<float>(input);
    output.clear();
    opus_encoder_ctl(opus_encoder, OPUS_SET_BITRATE(50000));
    std::vector<float> interlacedInput;
    std::vector<float> interlacedOutput;
    interlacedInput.resize(samples_per_frame * 2);
    interlacedOutput.resize(samples_per_frame * 2);
    std::array<unsigned char, 10000> encoded{};
    int corrupt_odds = 20;
    auto random = juce::Random();
    for (int i = 0; i < input.getNumSamples() - samples_per_frame; i += samples_per_frame) {
        float maxInputL = 0;
        float maxInputR = 0;
        for (int samp = 0; samp < samples_per_frame; ++samp) {
            interlacedInput[samp * 2] = input.getSample(0, i + samp);
            maxInputL = std::max(std::abs(interlacedInput[samp * 2]), maxInputL);
            interlacedInput[samp * 2 + 1] = input.getSample(0, i + samp + 1);
            maxInputR = std::max(std::abs(interlacedInput[samp * 2 + 1]), maxInputR);
        }
        auto encodeResult = 0;
        encodeResult = opus_encode_float(opus_encoder, &interlacedInput[0], samples_per_frame, &encoded[0], 10000);
        REQUIRE(encodeResult > 0);
        for (int samp = 0; samp < encodeResult; ++samp) {
            if (random.nextInt(corrupt_odds) == 0) {
                encoded[samp] ^= 1;
            }
        }
        opus_decode_float(opus_decoder, &encoded[0], encodeResult, &interlacedOutput[0], samples_per_frame, 0);

        auto maxOutputL = 0.f;
        auto maxOutputR = 0.f;
        for (int samp = 0; samp < samples_per_frame; ++samp) {
            maxOutputL = std::max(std::abs(interlacedOutput[samp * 2]), maxOutputL);
            maxOutputR = std::max(std::abs(interlacedOutput[samp * 2 + 1]), maxOutputR);
        }
        auto scalarL = maxOutputL > maxInputL ? maxInputL / maxOutputL : 1;
        auto scalarR = maxOutputR > maxInputR ? maxInputR / maxOutputR : 1;

        for (int samp = 0; samp < samples_per_frame; ++samp) {
            output.setSample(0, samp + i, interlacedOutput[samp * 2] * scalarL);
            output.setSample(1, samp + i + 1, interlacedOutput[samp * 2 + 1] * scalarR);
        }
    }
    writeOut(output, "50k_corrupt_opus.wav");
}

TEST_CASE("opus keep decoding same packet" "[opuskeepdecoding]")
{
    auto input = makeTestAudioBuffer();
    writeOut(input, "original.wav");
    auto error = 0;
    auto opus_encoder = opus_encoder_create(48000, 2, OPUS_APPLICATION_VOIP, &error);
    REQUIRE(error == 0);
    auto opus_decoder = opus_decoder_create(48000, 2, &error);
    REQUIRE(error == 0);
    int samples_per_frame = 10 /* ms / frame */ * 48 /* samples / ms */;
    REQUIRE(input.getNumChannels() == 2);
    auto buf = juce::AudioBuffer<float>(input.getNumChannels(), samples_per_frame);
    auto output = juce::AudioBuffer<float>(input);
    output.clear();
    opus_encoder_ctl(opus_encoder, OPUS_SET_BITRATE(50000));
    std::vector<float> interlacedInput;
    std::vector<float> interlacedOutput;
    interlacedInput.resize(samples_per_frame * 2);
    interlacedOutput.resize(samples_per_frame * 2);
    std::array<unsigned char, 10000> encoded{};
    auto encodeResult = 0;
    for (int i = 0; i < input.getNumSamples() - samples_per_frame; i += samples_per_frame) {
        if ((i / samples_per_frame) % 20 < 10) {
            for (int samp = 0; samp < samples_per_frame; ++samp) {
                interlacedInput[samp * 2] = input.getSample(0, i + samp);
                interlacedInput[samp * 2 + 1] = input.getSample(1, i + samp);
            }
            encodeResult = opus_encode_float(opus_encoder, &interlacedInput[0], samples_per_frame, &encoded[0], 10000);
            REQUIRE(encodeResult > 0);
        }
        auto decodeResult = opus_decode_float(opus_decoder, &encoded[0], encodeResult, &interlacedOutput[0], samples_per_frame, 0);
        REQUIRE(decodeResult == samples_per_frame);
        for (int samp = 0; samp < samples_per_frame; ++samp) {
            output.setSample(0, samp + i, interlacedOutput[samp * 2]);
            output.setSample(1, samp + i, interlacedOutput[samp * 2 + 1]);
        }
    }
    writeOut(output, "opusStuck.wav");

}

TEST_CASE("opus encode/decode", "[opusencodedecode]")
{
    auto gui = juce::ScopedJuceInitialiser_GUI {};

    MaimAudioProcessor testPlugin;
    testPlugin.prepareToPlay(44100, 512);
    auto& apvts = testPlugin.getValueTreeState();

    OpusController opusController(apvts);
    auto initResult = opusController.init(44100, 512, 100);
    REQUIRE(initResult == true);

    juce::AudioBuffer<float> samples(2,512);
    for (int i = 0; i < 10; ++i) {
        for (int samp = 0; samp < 512; ++samp) {
            samples.setSample(0, i, sin(static_cast<float>(samp) / 100.f));
            samples.setSample(1, i, sin(static_cast<float>(samp) / 100.f));
        }
        opusController.processBlock(samples);
    }
    samples.clear();
    opusController.processBlock(samples);
    REQUIRE(samples.findMinMax(0, 0, 512).getEnd() > 0);
}

TEST_CASE("turbo and packet repeat", "[turboandpacketrepeat]")
{
    // This lets us use JUCE's MessageManager without leaking.
    // PluginProcessor might need this if you use the APVTS for example.
    // You'll also need it for tests that rely on juce::Graphics, juce::Timer, etc.
    auto gui = juce::ScopedJuceInitialiser_GUI {};

    MaimAudioProcessor testPlugin;
    testPlugin.prepareToPlay(44100, 512);
    auto& apvts = testPlugin.getValueTreeState();
    *((juce::AudioParameterChoice*)apvts.getParameter(ENCODER_PARAM_ID)) = 2; // opus
    *((juce::AudioParameterFloat*)apvts.getParameter(TURBO_PARAM_ID)) = 0.8f;
    *((juce::AudioParameterBool*)apvts.getParameter(PACKET_LOSS_STICK_PARAM_ID)) = true;
    *((juce::AudioParameterFloat*)apvts.getParameter(PACKET_LOSS_PULSE_WIDTH_PARAM_ID)) = 0.2;
    *((juce::AudioParameterFloat*)apvts.getParameter(PACKET_LOSS_RATE_PARAM_ID)) = 0.4;

    juce::AudioBuffer<float> samples(2,512);
    for (int i = 0; i < 100; ++i) {
        for (int samp = 0; samp < 512; ++samp) {
            samples.setSample(0, i, sin(static_cast<float>(samp) / 100.f));
            samples.setSample(1, i, sin(static_cast<float>(samp) / 100.f));
        }
        auto midiBuffer = juce::MidiBuffer();
        testPlugin.processBlock(samples, midiBuffer);
    }
}


#ifdef PAMPLEJUCE_IPP
    #include <ipp.h>

TEST_CASE ("IPP version", "[ipp]")
{
    CHECK_THAT (ippsGetLibVersion()->Version, Catch::Matchers::Equals ("2021.11.0 (r0xcd107b02)"));
}
#endif