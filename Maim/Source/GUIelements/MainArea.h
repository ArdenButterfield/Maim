/*
  ==============================================================================

    MainArea.h
    Created: 4 Apr 2023 3:19:14pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "MDCTSection.h"
#include "PostSection.h"
#include "StageWindow.h"

//==============================================================================
/*
*/
class MainArea  : public juce::Component
{
public:
    MainArea(juce::AudioProcessorValueTreeState& p);
    ~MainArea() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    MDCTSection mdctSection;
    PostSection postSection;
    juce::AudioProcessorValueTreeState& parameters;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainArea)
};
