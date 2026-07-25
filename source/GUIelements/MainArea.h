/*
  ==============================================================================

    MainArea.h
    Created: 4 Apr 2023 3:19:14pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include "EncoderBitrateSection.h"
#include "MDCTGraphSection.h"
#include "MiscellaneaSection.h"
#include "OpusPacketLossSection.h"
#include "PostSection.h"
#include "PresetsSection.h"
#include "ReassignmentSection.h"
#include "StageWindow.h"
#include "TitlePanel.h"
#include "../PluginProcessor.h"
//==============================================================================
/*
*/
class MainArea  : public juce::Component, public juce::AudioProcessorValueTreeState::Listener
{
public:
    MainArea(MaimAudioProcessor& processor);
    ~MainArea() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void parameterChanged (const juce::String &parameterID, float newValue) override;

    EncoderBitrateSection psychoacousticSection;
    MiscellaneaSection miscellaneaSection;
    MDCTGraphSection mdctGraphSection;
    PostSection postSection;
    PresetsSection presetsSection;
    TitlePanel titlePanel;
    ReassignmentSection reassignmentSection;
    OpusPacketLossSection opusPacketLossSection;

    juce::AudioProcessorValueTreeState& parameters;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainArea)
};
