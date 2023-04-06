/*
  ==============================================================================

    DragBox.h
    Created: 6 Apr 2023 2:16:04pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <memory>

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
//==============================================================================
/*
*/
class DragBox  : public juce::Component, public juce::AudioProcessorValueTreeState::Listener
{
public:
    DragBox(juce::AudioProcessorValueTreeState& p,
            const juce::String& xParamID,
            const juce::String& yParamID);
    ~DragBox() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Point<int> thumb;
    const int thumbDrawRadius = 5;
    const int thumbSensitivityRadius = 10;
    bool thumbHovered;
    bool thumbDragged;
    
    const juce::String xParamID;
    const juce::String yParamID;
    std::unique_ptr<juce::Slider> xSlider;
    std::unique_ptr<juce::Slider> ySlider;
    std::unique_ptr<SliderAttachment> xAttachment;
    std::unique_ptr<SliderAttachment> yAttachment;
    
    juce::AudioProcessorValueTreeState& parameters;
    void parameterChanged (const juce::String &parameterID, float newValue) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DragBox)
};
