/*
  ==============================================================================

    MP3ControllerManager.h
    Created: 10 Apr 2023 5:05:43pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>
#include <cmath>

#include "LameController.h"
#include "BladeController.h"

#define NUM_REASSIGNMENT_BANDS 20

class MP3ControllerManager : public juce::AudioProcessorValueTreeState::Listener,
public juce::Timer
{
public:
    MP3ControllerManager(int samplerate,
                          int initialBitrate,
                          int samplesPerBlock,
                          juce::AudioProcessorValueTreeState& parameters);
    ~MP3ControllerManager();

    void changeBitrate(int newBitrate);
    void processBlock(juce::AudioBuffer<float>& buffer);
    
    void updateParameters(bool updateOffController=false);
    int getBitrate();
    
    float* getPsychoanalEnergy();
    float* getPsychoanalThreshold();
    
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
    
    bool wantingToSwitch;
    int currentBitrate;
    const int samplerate;
    const int samplesPerBlock;
    
    const int blocksBeforeSwitch;
    int switchCountdown;
    
    std::array<BladeController, 2> controllers;
    BladeController* currentController;
    BladeController* offController;
    
    std::array<juce::AudioParameterInt*, 20> bandReassignmentParameters;
    juce::AudioProcessorValueTreeState& parameters;
    
};