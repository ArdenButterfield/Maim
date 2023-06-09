/*
  ==============================================================================

    PostSection.h
    Created: 7 Apr 2023 9:21:27am
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include "StageWindow.h"

//==============================================================================
/*
*/
class PostSection  : public StageWindow
{
public:
    PostSection(juce::AudioProcessorValueTreeState& p);
    void resized() override;
private:
    juce::Slider driveSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    
    juce::AudioProcessorValueTreeState::SliderAttachment driveAttachment {
        parameters,
        "drive",
        driveSlider
    };
    
    juce::Slider makeupSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    
    juce::AudioProcessorValueTreeState::SliderAttachment makeupAttachment {
        parameters,
        "makeupgain",
        makeupSlider
    };

    juce::Slider encoderSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    
    juce::AudioProcessorValueTreeState::SliderAttachment encoderAttachment {
        parameters,
        "encoder",
        encoderSlider
    };
    
    juce::Slider bitrateSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    
    juce::AudioProcessorValueTreeState::SliderAttachment bitrateAttachment {
        parameters,
        "bitrate",
        bitrateSlider
    };
    
    juce::Slider postFilterSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    
    juce::AudioProcessorValueTreeState::SliderAttachment postFilterAttatchment {
        parameters,
        "lopass",
        postFilterSlider
    };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PostSection)
};
