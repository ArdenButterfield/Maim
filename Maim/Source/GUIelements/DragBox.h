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
#include <cmath>

typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
//==============================================================================
/*
*/
class DragBox  :
        public juce::Component,
        public juce::AudioProcessorValueTreeState::Listener
{
public:
    DragBox(juce::AudioProcessorValueTreeState& p,
            const juce::String& xParamID,
            const juce::String& yParamID);
    ~DragBox() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;

    void mouseMove (const juce::MouseEvent &event) override;
    void mouseDrag (const juce::MouseEvent &event) override;
    void mouseDoubleClick (const juce::MouseEvent &event) override;

private:
    juce::Point<int> thumb;
    const int thumbDrawRadius = 5;
    const int thumbSensitivityRadius = 10;
    
    const float gridStep = 2.0f;
    std::vector<int> horizontalGridlines;
    std::vector<int> verticalGridlines;
    void calculateGridLines(float minVal, float maxVal, float step, float outMax, std::vector<int>* v);
    
    void drawGridlines(juce::Graphics&);
    
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
