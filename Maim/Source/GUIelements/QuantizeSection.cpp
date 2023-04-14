/*
  ==============================================================================

    QuantizeSection.cpp
    Created: 8 Apr 2023 1:50:16pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include <JuceHeader.h>
#include "QuantizeSection.h"

//==============================================================================
QuantizeSection::QuantizeSection(juce::AudioProcessorValueTreeState& p)
: StageWindow(p), arrayAssigner(p, 20, 20)
{
    addAndMakeVisible(arrayAssigner);
    addAndMakeVisible(squishSlider);
    addAndMakeVisible(MDCTfeedbackSlider);
}

void QuantizeSection::resized()
{
    squishSlider.setBounds(getLocalBounds().withTrimmedLeft(200).withTrimmedTop(100));
    MDCTfeedbackSlider.setBounds(getLocalBounds().withTrimmedLeft(200).withHeight(100));
    arrayAssigner.setBounds(getLocalBounds().withWidth(200));

}
