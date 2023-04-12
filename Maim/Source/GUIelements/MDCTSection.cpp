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
    addAndMakeVisible(mdctPostPitchShiftSlider);
    addAndMakeVisible(mdctPostAmpShiftSlider);
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
    mdctPostPitchShiftSlider.setBounds(otherpart.withTrimmedTop(halfHeight).withTrimmedLeft(halfWidth).withHeight(70));
    mdctPostAmpShiftSlider.setBounds(otherpart.withTrimmedTop(halfHeight).withTrimmedLeft(halfWidth).withTrimmedTop(70));
    mdctWindowIncrementSlider.setBounds(otherpart.withTrimmedTop(halfHeight).withTrimmedRight(halfWidth).withHeight(70));
}
