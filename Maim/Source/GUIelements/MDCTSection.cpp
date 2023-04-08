/*
  ==============================================================================

    MDCTSection.cpp
    Created: 7 Apr 2023 9:21:14am
    Author:  Arden Butterfield

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MDCTSection.h"


MDCTSection::MDCTSection(juce::AudioProcessorValueTreeState& p)
        : StageWindow(p),
            butterflyDragBox(p, "butterflystandard", "butterflycrossed")
{
    addAndMakeVisible(mdctBandStepSlider);
    addAndMakeVisible(mdctBandInvertSlider);
    addAndMakeVisible(mdctPostShiftSlider);
    addAndMakeVisible(mdctWindowIncrementSlider);
    addAndMakeVisible(butterflyDragBox);
    addAndMakeVisible(mdctSampIncrementSlider);
}


void MDCTSection::resized()
{
    butterflyDragBox.setBounds(getLocalBounds().withTrimmedRight(250));
    auto otherpart = getLocalBounds().withTrimmedLeft(getWidth()-250);
    int halfHeight = otherpart.getHeight() / 2;
    int halfWidth = otherpart.getWidth() / 2;
    mdctBandStepSlider.setBounds(otherpart.withTrimmedBottom(halfHeight).withTrimmedRight(halfWidth));
    mdctBandInvertSlider.setBounds(otherpart.withTrimmedBottom(halfHeight).withTrimmedLeft(halfWidth));
    mdctPostShiftSlider.setBounds(otherpart.withTrimmedTop(halfHeight).withTrimmedLeft(halfWidth));
    mdctWindowIncrementSlider.setBounds(otherpart.withTrimmedTop(halfHeight).withTrimmedRight(halfWidth).withHeight(70));
    mdctSampIncrementSlider.setBounds(otherpart.withTrimmedTop(halfHeight).withTrimmedRight(halfWidth).withTrimmedTop(70));
}
