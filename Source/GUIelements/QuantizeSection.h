/*
  ==============================================================================

    QuantizeSection.h
    Created: 8 Apr 2023 1:50:16pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include "StageWindow.h"
#include "ArrayAssigner.h"
#include "MDCTGraph.h"

//==============================================================================
/*
*/
class QuantizeSection  : public StageWindow
{
public:
    QuantizeSection(juce::AudioProcessorValueTreeState& p);
    void resized() override;

private:
    juce::Slider MDCTfeedbackSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    juce::AudioProcessorValueTreeState::SliderAttachment MDCTfeedbackAttachment {
        parameters,
        "mdctfeedback",
        MDCTfeedbackSlider
    };

    MDCTGraph mdctGraph;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuantizeSection)
};
