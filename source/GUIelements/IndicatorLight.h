/*
  ==============================================================================

    IndicatorLight.h
    Created: 14 Apr 2023 2:04:50pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>
//==============================================================================
/*
*/
class IndicatorLight  : public juce::Component, public juce::Timer
{
public:
    IndicatorLight();
    ~IndicatorLight() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void triggerLight();
private:
    std::atomic<bool> isOn;
    const juce::Colour onColour = juce::Colours::yellow;
    const juce::Colour offColour = juce::Colours::black;
    
    void timerCallback() override;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IndicatorLight)
};
