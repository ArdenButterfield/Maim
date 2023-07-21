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
    void paint (juce::Graphics& g);
    void resized() override;
private:
    juce::Rectangle<int> driveSection;
    juce::Rectangle<int> bitrateSection;
    juce::Rectangle<int> filterSection;
    juce::Rectangle<int> makeupSection;

    NamedRotarySlider driveSlider;
    NamedRotarySlider encoderSlider;
    NamedRotarySlider bitrateSlider;
    // NamedRotarySlider squishSlider;
    NamedRotarySlider postFilterSlider;
    NamedRotarySlider makeupSlider;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PostSection)
};
