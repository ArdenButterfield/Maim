/*
  ==============================================================================

    MDCTSection.h
    Created: 7 Apr 2023 9:21:14am
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include "StageWindow.h"
#include "DragBox.h"

//==============================================================================
/*
*/
class MDCTSection  : public StageWindow
{
public:
    MDCTSection(juce::AudioProcessorValueTreeState& p);
    void resized() override;
private:
    DragBox butterflyDragBox;
    
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
    
    juce::Slider mdctPostPitchShiftSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    juce::AudioProcessorValueTreeState::SliderAttachment mdctPostPitchShiftAttachment {
        parameters,
        "mdctposthshift",
        mdctPostPitchShiftSlider
    };
    
    juce::Slider mdctPostAmpShiftSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    juce::AudioProcessorValueTreeState::SliderAttachment mdctPostAmpShiftAttachment {
        parameters,
        "mdctpostvshift",
        mdctPostAmpShiftSlider
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
