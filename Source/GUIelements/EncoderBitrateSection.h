/*
  ==============================================================================

    EncoderBitrateSection.h
    Created: 8 Apr 2023 1:50:05pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include "StageWindow.h"
#include "PsychoanalGraph.h"
#include "IndicatorLight.h"
#include "NamedRotarySlider.h"

//==============================================================================
/*
*/
class EncoderBitrateSection : public StageWindow, public juce::ValueTree::Listener
{
public:
    EncoderBitrateSection (juce::AudioProcessorValueTreeState& p);
    ~EncoderBitrateSection();
    void resized() override;

private:
    void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                  const juce::Identifier &property) override;
    NamedRotarySlider biasSlider;

    juce::ToggleButton encoderButton {"Blade Encoder|Lame Encoder"};
    const juce::AudioProcessorValueTreeState::ButtonAttachment encoderAttachment;
    NamedRotarySlider bitrateSlider;
    NamedRotarySlider squishSlider;

    PsychoanalGraph psychoanalGraph;
    IndicatorLight shortFrameIndicator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EncoderBitrateSection)
};
