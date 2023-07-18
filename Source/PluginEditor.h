/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include "PluginProcessor.h"
#include "GUIelements/MainArea.h"
#include "GUIelements/DragBox.h"
#include "MaimLookAndFeel.h"

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
    MaimAudioProcessor* getAudioProcessor();
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    MaimAudioProcessor& audioProcessor;
    
    MainArea mainArea;
    DragBox dragBox;

    MaimLookAndFeel maimLookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MaimAudioProcessorEditor)
};
