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
#include "Mp3OnlyLabel.h"
#include "../parameterIds.h"
//==============================================================================
/*
*/
class MDCTGraphSection : public StageWindow, public juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit MDCTGraphSection (juce::AudioProcessorValueTreeState& p);
    ~MDCTGraphSection() override {
        parameters.removeParameterListener(ENCODER_PARAM_ID, this);
    }
    void parameterChanged(const juce::String &parameterID, float newValue) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& parameters;
    Mp3OnlyLabel mp3OnlyLabel;

    const juce::Font sectionNameFont = juce::Font(MaimLookAndFeel().main_font).withHeight(20.f);
    juce::Label sectionName;
    juce::Label feedbackName;

    DemureSlider MDCTfeedbackSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    juce::AudioProcessorValueTreeState::SliderAttachment MDCTfeedbackAttachment {
        parameters,
        MDCT_FEEDBACK_PARAM_ID,
        MDCTfeedbackSlider
    };

    DemureSlider mdctPostPitchShiftSlider {
        juce::Slider::LinearHorizontal,
        juce::Slider::TextBoxAbove
    };

    juce::AudioProcessorValueTreeState::SliderAttachment mdctPostPitchShiftAttachment {
        parameters,
        MDCT_PITCH_SHIFT_PARAM_ID,
        mdctPostPitchShiftSlider
    };

    DemureSlider mdctPostAmpShiftSlider {
        juce::Slider::LinearVertical,
        juce::Slider::TextBoxRight
    };
    juce::AudioProcessorValueTreeState::SliderAttachment mdctPostAmpShiftAttachment {
        parameters,
        MDCT_AMPLITUDE_SHIFT_PARAM_ID,
        mdctPostAmpShiftSlider
    };


    MDCTGraph mdctGraph;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MDCTGraphSection)
};
