/*
  ==============================================================================

    PsychoacousticSection.h
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
class PsychoacousticSection  : public StageWindow, public juce::ValueTree::Listener
{
public:
    PsychoacousticSection(juce::AudioProcessorValueTreeState& p);
    ~PsychoacousticSection();
    void resized() override;

private:
    void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                  const juce::Identifier &property) override;
    NamedRotarySlider biasSlider;
/*
    juce::Slider biasSlider {
        juce::Slider::RotaryVerticalDrag,
        juce::Slider::TextBoxBelow
    };
    juce::AudioProcessorValueTreeState::SliderAttachment biasAttachment {
        parameters,
        "thresholdbias",
        biasSlider
    };
*/

    PsychoanalGraph psychoanalGraph;
    IndicatorLight shortFrameIndicator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PsychoacousticSection)
};
