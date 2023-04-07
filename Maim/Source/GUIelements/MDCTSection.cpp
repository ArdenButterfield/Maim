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
        : StageWindow(p)
{
    addAndMakeVisible(mdctBandStepSlider);
    addAndMakeVisible(mdctBandInvertSlider);
    addAndMakeVisible(mdctPostShiftSlider);
    addAndMakeVisible(mdctWindowIncrementSlider);
}


void MDCTSection::resized()
{
    int halfHeight = getHeight() / 2;
    int halfWidth = getWidth() / 2;
    mdctBandStepSlider.setBounds(getLocalBounds().withTrimmedBottom(halfHeight).withTrimmedRight(halfWidth));
    mdctBandInvertSlider.setBounds(getLocalBounds().withTrimmedBottom(halfHeight).withTrimmedLeft(halfWidth));
    mdctPostShiftSlider.setBounds(getLocalBounds().withTrimmedTop(halfHeight).withTrimmedLeft(halfWidth));
    mdctWindowIncrementSlider.setBounds(getLocalBounds().withTrimmedTop(halfHeight).withTrimmedRight(halfWidth));

}
