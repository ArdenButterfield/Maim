/*
  ==============================================================================

    QuantizeSection.h
    Created: 8 Apr 2023 1:50:16pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "StageWindow.h"
#include "ArrayAssigner.h"

//==============================================================================
/*
*/
class QuantizeSection  : public StageWindow
{
public:
    QuantizeSection(juce::AudioProcessorValueTreeState& p);
    void resized() override;

private:
    ArrayAssigner arrayAssigner;
    
    juce::Slider squishSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    juce::AudioProcessorValueTreeState::SliderAttachment squishAttachment {
        parameters,
        "bitratesquish",
        squishSlider
    };

    juce::Slider MDCTfeedbackSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    juce::AudioProcessorValueTreeState::SliderAttachment MDCTfeedbackAttachment {
        parameters,
        "mdctfeedback",
        MDCTfeedbackSlider
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuantizeSection)
};
