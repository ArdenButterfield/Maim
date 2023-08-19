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
#include "NamedRotarySlider.h"
#include "TiltGraph.h"

//==============================================================================
/*
*/
class MDCTSection  : public StageWindow, public juce::AudioProcessorValueTreeState::Listener
{
public:
    MDCTSection(juce::AudioProcessorValueTreeState& p);
    ~MDCTSection();
    void resized() override;
private:
    const juce::Font sectionNameFont = juce::Font(MaimLookAndFeel().main_font).withHeight(20.f);
    juce::Label sectionName;
    DragBox butterflyDragBox;
    void parameterChanged (const juce::String &parameterID, float newValue) override;

    NamedRotarySlider mdctBandStepSlider;

/*
    juce::Slider mdctBandStepSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    
    juce::AudioProcessorValueTreeState::SliderAttachment mdctBandStepAttachment {
        parameters,
        "mdctstep",
        mdctBandStepSlider
    };
*/

    NamedRotarySlider mdctWindowIncrementSlider;
/*
    juce::Slider mdctWindowIncrementSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    juce::AudioProcessorValueTreeState::SliderAttachment mdctWindowIncrementAttachment {
        parameters,
        "mdctwindowincr",
        mdctWindowIncrementSlider
    };
*/
    TiltGraph tiltGraph;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MDCTSection)
};
