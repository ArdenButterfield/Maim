/*
  ==============================================================================

    MiscellaneaSection.h
    Created: 7 Apr 2023 9:21:14am
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include "StageWindow.h"
#include "ButterflyDragBox.h"
#include "SquishFlipDragBox.h"
#include "NamedRotarySlider.h"
#include "TiltGraph.h"

//==============================================================================
/*
*/
class MiscellaneaSection : public StageWindow, public juce::AudioProcessorValueTreeState::Listener
{
public:
    MiscellaneaSection (juce::AudioProcessorValueTreeState& p);
    ~MiscellaneaSection();
    void resized() override;
    void paint (juce::Graphics&) override;
private:
    void parameterChanged (const juce::String &parameterID, float newValue) override;
    void mouseMove (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;

    const juce::Font sectionNameFont = juce::Font(MaimLookAndFeel().main_font).withHeight(20.f);
    const juce::Font lameLabelFont = juce::Font(MaimLookAndFeel().main_font).withHeight(10.f);
    juce::Label sectionName;
    juce::Label lameOnlyLabel;
    juce::Label butterflyLabel;
    ButterflyDragBox butterflyDragBox;
    SquishFlipDragBox mdctDragBox;
    juce::Rectangle<int> lameOnlySection;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MiscellaneaSection)
};
