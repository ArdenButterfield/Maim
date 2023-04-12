/*
  ==============================================================================

    LameControllerManager.h
    Created: 10 Apr 2023 5:05:43pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>

#include "LameController.h"

class LameControllerManager
{
public:
    LameControllerManager(int samplerate, int initialBitrate, int samplesPerBlock);
    ~LameControllerManager();

    void changeBitrate(int newBitrate);
    void processBlock(juce::AudioBuffer<float>& buffer);
    
    void updateParameters(juce::AudioProcessorValueTreeState& parameters,
                          std::array<juce::AudioParameterInt*, 20>* bandReassignmentParameters,
                          bool updateOffController=false);
    int getBitrate();
    
private:
    bool wantingToSwitch;
    int currentBitrate;
    const int samplerate;
    const int samplesPerBlock;
    
    const int blocksBeforeSwitch;
    int switchCountdown;
    
    std::array<LameController, 2> controllers;
    LameController* currentController;
    LameController* offController;
};
