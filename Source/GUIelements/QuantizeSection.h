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
#include "DemureSlider.h"

//==============================================================================
/*
*/
class QuantizeSection  : public StageWindow
{
public:
    QuantizeSection(juce::AudioProcessorValueTreeState& p);
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void resized() override;

private:
    DemureSlider MDCTfeedbackSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    juce::AudioProcessorValueTreeState::SliderAttachment MDCTfeedbackAttachment {
        parameters,
        "mdctfeedback",
        MDCTfeedbackSlider
    };
    DemureSlider mdctPostPitchShiftSlider {
        juce::Slider::LinearHorizontal,
        juce::Slider::TextBoxAbove
    };


    juce::AudioProcessorValueTreeState::SliderAttachment mdctPostPitchShiftAttachment {
        parameters,
        "mdctposthshift",
        mdctPostPitchShiftSlider
    };

    DemureSlider mdctPostAmpShiftSlider {
        juce::Slider::LinearVertical,
        juce::Slider::TextBoxRight
    };
    juce::AudioProcessorValueTreeState::SliderAttachment mdctPostAmpShiftAttachment {
        parameters,
        "mdctpostvshift",
        mdctPostAmpShiftSlider
    };


    MDCTGraph mdctGraph;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuantizeSection)
};
