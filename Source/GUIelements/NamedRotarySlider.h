//
// Created by arden on 7/20/23.
//

#ifndef MAIM_NAMEDROTARYSLIDER_H
#define MAIM_NAMEDROTARYSLIDER_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include "MaimLookAndFeel.h"


class NamedRotarySlider : public juce::Component
{
public:
    NamedRotarySlider(juce::AudioProcessorValueTreeState& p, juce::String parameterName, juce::String displayName);
    void resized() override;
    void paint(juce::Graphics &g) override;
private:
    juce::Slider slider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    const juce::AudioProcessorValueTreeState::SliderAttachment attachment;
    const juce::String displayName;
    int nameHeight;

    const juce::Font nameFont = juce::Font(MaimLookAndFeel().main_font).withHeight(14.f);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NamedRotarySlider)

};

#endif //MAIM_NAMEDROTARYSLIDER_H
