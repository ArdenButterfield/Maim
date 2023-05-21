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

#include "StageWindow.h"
#include "PsychoacousticSection.h"
#include "MDCTSection.h"
#include "QuantizeSection.h"
#include "DecodeSection.h"
#include "PostSection.h"

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
    PsychoacousticSection psychoacousticSection;
    MDCTSection mdctSection;
    QuantizeSection quantizeSection;
    DecodeSection decodeSection;
    PostSection postSection;
    
    juce::AudioProcessorValueTreeState& parameters;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainArea)
};
