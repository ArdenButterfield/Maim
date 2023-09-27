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
#include "PostSection.h"
#include "ReassignmentSection.h"
#include "StageWindow.h"
#include "TitlePanel.h"
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
    EncoderBitrateSection psychoacousticSection;
    MiscellaneaSection miscellaneaSection;
    MDCTGraphSection mdctGraphSection;
    PostSection postSection;
    TitlePanel titlePanel;
    ReassignmentSection reassignmentSection;

    juce::AudioProcessorValueTreeState& parameters;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainArea)
};
