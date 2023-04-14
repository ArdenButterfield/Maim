/*
  ==============================================================================

    PsychoacousticSection.h
    Created: 8 Apr 2023 1:50:05pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "StageWindow.h"
#include "PsychoanalGraph.h"

//==============================================================================
/*
*/
class PsychoacousticSection  : public StageWindow
{
public:
    PsychoacousticSection(juce::AudioProcessorValueTreeState& p);
    void resized() override;

private:
    juce::Slider biasSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    juce::AudioProcessorValueTreeState::SliderAttachment biasAttachment {
        parameters,
        "thresholdbias",
        biasSlider
    };
    
    PsychoanalGraph psychoanalGraph;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PsychoacousticSection)
};
