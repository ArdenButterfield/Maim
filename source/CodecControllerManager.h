//
// Created by arden on 1/28/25.
//

#ifndef MAIM_CODECCONTROLLERMANAGER_H
#define MAIM_CODECCONTROLLERMANAGER_H

#include "juce_audio_basics/juce_audio_basics.h"
#include "Mp3ControllerManager.h"
#include "CodecControllers/OpusController.h"
class CodecControllerManager : public juce::AudioProcessorValueTreeState::Listener
{
public:
    CodecControllerManager(juce::AudioProcessorValueTreeState& parameters) : mp3ControllerManager(parameters), params(parameters) {
        auto encoder = ((juce::AudioParameterChoice*)
                        parameters.getParameter("encoder"))->getIndex();
        if (encoder == 2 /* opus */ ) {
            encoderType = use_opus;
        } else {
            encoderType = use_mp3;
        }
    }
    ~CodecControllerManager();
    void initialize(int samplerate, int initialBitrate, int samplesPerBlock) {
        opusController.init(samplerate, samplesPerBlock, initialBitrate);
        mp3ControllerManager.initialize(samplerate, initialBitrate, samplesPerBlock);
    }
    
    void processBlock(juce::AudioBuffer<float>& block);
    void releaseResources();
    void parameterChanged(const juce::String &parameterID, float newValue) override {
        if (parameterID == "encoder") {
            auto encoder = ((juce::AudioParameterChoice*)
                                params.getParameter("encoder"))->getIndex();
            if (encoder == 2 /* opus */ ) {
                encoderType = use_opus;
            } else {
                encoderType = use_mp3;
            }
            switchingEncoderType = true;
        }
    }
private:
    bool parametersNeedUpdating;
    Mp3ControllerManager mp3ControllerManager;
    OpusController opusController;
    enum EncoderType {
        use_mp3,
        use_opus
    };
    EncoderType encoderType;
    bool switchingEncoderType;
    juce::AudioProcessorValueTreeState& params;
};

#endif //MAIM_CODECCONTROLLERMANAGER_H
