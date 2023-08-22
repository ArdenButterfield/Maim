/*
  ==============================================================================

    MDCTGraphSection.h
    Created: 8 Apr 2023 1:50:16pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include "DemureSlider.h"
#include "MDCTGraph.h"
#include "ReassignmentSection.h"
#include "StageWindow.h"

//==============================================================================
/*
*/
class MDCTGraphSection : public StageWindow
{
public:
    MDCTGraphSection (juce::AudioProcessorValueTreeState& p);
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void resized() override;

private:
    const juce::Font sectionNameFont = juce::Font(MaimLookAndFeel().main_font).withHeight(20.f);
    juce::Label sectionName;
    juce::Label feedbackName;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MDCTGraphSection)
};
