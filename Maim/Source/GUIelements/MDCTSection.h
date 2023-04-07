/*
  ==============================================================================

    MDCTSection.h
    Created: 7 Apr 2023 9:21:14am
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "StageWindow.h"

//==============================================================================
/*
*/
class MDCTSection  : public StageWindow
{
public:
    MDCTSection(juce::AudioProcessorValueTreeState& p);
    void resized() override;
private:
    juce::Slider mdctBandStepSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    
    juce::AudioProcessorValueTreeState::SliderAttachment mdctBandStepAttachment {
        parameters,
        "mdctstep",
        mdctBandStepSlider
    };
    
    juce::Slider mdctBandInvertSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    
    juce::AudioProcessorValueTreeState::SliderAttachment mdctBandInvertAttachment {
        parameters,
        "mdctinvert",
        mdctBandInvertSlider
    };
    
    juce::Slider mdctPostShiftSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    juce::AudioProcessorValueTreeState::SliderAttachment mdctPostShiftAttachment {
        parameters,
        "mdctpostshift",
        mdctPostShiftSlider
    };
    
    juce::Slider mdctWindowIncrementSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    juce::AudioProcessorValueTreeState::SliderAttachment mdctWindowIncrementAttachment {
        parameters,
        "mdctwindowincr",
        mdctWindowIncrementSlider
    };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MDCTSection)
};
