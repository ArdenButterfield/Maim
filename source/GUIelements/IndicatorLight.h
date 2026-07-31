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
#include <functional>
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
    // When a source is set, the light continuously reflects the source's
    // return value instead of flashing on trigger and resetting each tick.
    void setSource(std::function<bool()> newSource);
private:
    std::atomic<bool> isOn;
    std::function<bool()> source;
    const juce::Colour onColour = juce::Colours::yellow;
    const juce::Colour offColour = juce::Colours::black;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IndicatorLight)
};
