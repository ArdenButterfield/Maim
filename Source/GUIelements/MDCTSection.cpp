/*
  ==============================================================================

    MDCTSection.cpp
    Created: 7 Apr 2023 9:21:14am
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "MDCTSection.h"


MDCTSection::MDCTSection(juce::AudioProcessorValueTreeState& p)
        : StageWindow(p),
            butterflyDragBox(p, "butterflystandard", "butterflycrossed")
{
    addAndMakeVisible(mdctBandStepSlider);
    addAndMakeVisible(mdctBandInvertSlider);
    addAndMakeVisible(mdctWindowIncrementSlider);
    addAndMakeVisible(butterflyDragBox);
}


void MDCTSection::resized()
{
    butterflyDragBox.setBounds(getLocalBounds().withTrimmedRight(250));
    auto otherpart = getLocalBounds().withTrimmedLeft(getWidth()-250);
    int halfHeight = otherpart.getHeight() / 2;
    int halfWidth = otherpart.getWidth() / 2;
    mdctBandStepSlider.setBounds(otherpart.withTrimmedBottom(halfHeight).withTrimmedRight(halfWidth));
    mdctBandInvertSlider.setBounds(otherpart.withTrimmedBottom(halfHeight).withTrimmedLeft(halfWidth));
    mdctWindowIncrementSlider.setBounds(otherpart.withTrimmedTop(halfHeight).withTrimmedRight(halfWidth).withHeight(70));
}
