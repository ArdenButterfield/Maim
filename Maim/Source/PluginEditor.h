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

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MaimAudioProcessorEditor)
};
