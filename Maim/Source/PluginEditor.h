/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUIelements/MainArea.h"
#include "GUIelements/DragBox.h"

//==============================================================================
typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;


class MaimAudioProcessorEditor  :
        public juce::AudioProcessorEditor
{
public:
    MaimAudioProcessorEditor (MaimAudioProcessor&);
    ~MaimAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MaimAudioProcessor& audioProcessor;
    
    MainArea mainArea;
    DragBox dragBox;

    juce::Slider mdctBandStepSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    
    SliderAttachment mdctBandStepAttachment {
        audioProcessor.getValueTreeState(),
        "mdctstep",
        mdctBandStepSlider
    };
    
    juce::Slider mdctBandInvertSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    
    SliderAttachment mdctBandInvertAttachment {
        audioProcessor.getValueTreeState(),
        "mdctinvert",
        mdctBandInvertSlider
    };
    
    juce::Slider postFilterSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    
    SliderAttachment postFilterAttatchment {
        audioProcessor.getValueTreeState(),
        "lopass",
        postFilterSlider
    };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MaimAudioProcessorEditor)
};
