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
#include "NamedRotarySlider.h"

//==============================================================================
/*
*/
class PostSection  : public StageWindow
{
public:
    PostSection(juce::AudioProcessorValueTreeState& p);
    void paint (juce::Graphics& g) override;
    void resized() override;
private:
    NamedRotarySlider driveSlider;
    NamedRotarySlider loSlider;
    NamedRotarySlider hiSlider;
    NamedRotarySlider makeupSlider;
    NamedRotarySlider mixSlider;

    std::array<NamedRotarySlider*, 5> sliders = {&driveSlider, &hiSlider, &loSlider, &makeupSlider, &mixSlider};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PostSection)
};
