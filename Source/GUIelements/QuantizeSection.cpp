/*
  ==============================================================================

    QuantizeSection.cpp
    Created: 8 Apr 2023 1:50:16pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "QuantizeSection.h"

//==============================================================================
QuantizeSection::QuantizeSection(juce::AudioProcessorValueTreeState& p)
: StageWindow(p)
{
    addAndMakeVisible(MDCTfeedbackSlider);
}

void QuantizeSection::resized()
{
    MDCTfeedbackSlider.setBounds(getLocalBounds().withTrimmedLeft(200).withHeight(100));

}
