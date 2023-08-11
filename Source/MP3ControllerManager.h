/*
  ==============================================================================

    MP3ControllerManager.h
    Created: 10 Apr 2023 5:05:43pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include <array>
#include <cmath>

#include "LameController.h"
#include "BladeController.h"

#define NUM_REASSIGNMENT_BANDS 20

enum Encoder {
    blade = 0,
    lame = 1
};

class MP3ControllerManager : public juce::AudioProcessorValueTreeState::Listener,
public juce::Timer
{
public:
    MP3ControllerManager(juce::AudioProcessorValueTreeState& parameters);
    ~MP3ControllerManager();


    void initialize(int samplerate, int initialBitrate, int samplesPerBlock);

    void processBlock(juce::AudioBuffer<float>& buffer);
    
    int getBitrate();
    
    float* getPsychoanalEnergy();
    float* getPsychoanalThreshold();
    float* getMDCTpreBend();
    float* getMDCTpostBend();
    
    static constexpr std::array<int, 17> bitrates {
        8,
        16,
        24,
        32,
        40,
        48,
        56,
        64,
        80,
        96,
        112,
        128,
        160,
        192,
        224,
        256,
        320
    };

    
private:
    void timerCallback() override;
    
    std::atomic<bool> parametersNeedUpdating;
    void parameterChanged (const juce::String &parameterID, float newValue) override;
    void updateParameters(bool updateOffController=false);
    void changeController(int bitrate, Encoder encoder);
    bool wantingToSwitch;
    
    int currentBitrate;
    int desiredBitrate;
    Encoder currentEncoder;
    Encoder desiredEncoder;
    
    int currentControllerIndex;
    
    int samplerate;
    int samplesPerBlock;
    
    int blocksBeforeSwitch;
    int switchCountdown;
    
    
    
    std::array<LameController, 2> lameControllers;
    std::array<BladeController, 2> bladeControllers;
    MP3Controller* currentController;
    MP3Controller* offController;
    
    std::array<juce::AudioParameterInt*, 20> bandReassignmentParameters;
    juce::AudioProcessorValueTreeState& parameters;
    
};
