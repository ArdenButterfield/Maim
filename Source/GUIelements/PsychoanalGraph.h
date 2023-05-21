/*
  ==============================================================================

    PsychoanalGraph.h
    Created: 12 Apr 2023 3:55:31pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include <array>

#include "LineGraph.h"


//==============================================================================
/*
*/
class PsychoanalGraph  : public juce::Component, public juce::ValueTree::Listener
{
public:
    PsychoanalGraph(juce::AudioProcessorValueTreeState& p);
    ~PsychoanalGraph() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                  const juce::Identifier &property) override;
    juce::AudioProcessorValueTreeState& parameters;
    std::array<float, 22> thresholdVals;
    std::array<float, 22> energyVals;
    LineGraph<float> threshold;
    LineGraph<float> energy;
    
    const int numBands = 22;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PsychoanalGraph)
};
