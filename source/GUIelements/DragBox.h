/*
  ==============================================================================

    DragBox.h
    Created: 6 Apr 2023 2:16:04pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include <memory>
#include <cmath>


typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
//==============================================================================
/*
*/
class DragBox  :
        public juce::Component,
        public juce::Timer,
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
    void mouseDown(const juce::MouseEvent &event) override;

protected:
    static float rescaleRange(float v,
         float oldMin,
         float oldMax,
         float newMin,
         float newMax);
    virtual void drawBackground(juce::Graphics& g, int x, int y) {}
    virtual void calculationsOnResize() {}
    virtual juce::Colour getThumbFillColour(int x, int y) {return {};}
    virtual juce::Colour getOutlineColour(int x, int y) {return {};}
    virtual juce::Colour getBackgroundColour(int x, int y) {return {};}
    void timerCallback() override;
    std::atomic<bool> needsRepainting;
    
    juce::Point<int> thumb;
    const int thumbDrawRadius = 7;
    const int thumbSensitivityRadius = thumbDrawRadius;

    juce::Rectangle<int> box;
    juce::Rectangle<int> activeZone;
    

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
