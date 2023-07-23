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
: StageWindow(p), mdctGraph(p)
{
    // addAndMakeVisible(MDCTfeedbackSlider);
    addAndMakeVisible(mdctGraph);
}

void QuantizeSection::resized()
{
    MDCTfeedbackSlider.setBounds(getLocalBounds().withTrimmedLeft(200).withHeight(100));
    mdctGraph.setBounds(getLocalBounds().withTrimmedTop(20).withTrimmedLeft(10).withTrimmedBottom(10).withTrimmedRight(10));
}
