/*
  ==============================================================================

    PostSection.cpp
    Created: 7 Apr 2023 9:21:27am
    Author:  Arden Butterfield

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PostSection.h"

//==============================================================================

PostSection::PostSection(juce::AudioProcessorValueTreeState& p) : StageWindow(p) {
    addAndMakeVisible(postFilterSlider);
    addAndMakeVisible(bitrateSlider);
    addAndMakeVisible(driveSlider);
    addAndMakeVisible(makeupSlider);
}

void PostSection::resized()
{
    int knobWidth = getWidth() / 4;
    driveSlider.setBounds(getLocalBounds().withWidth(knobWidth));
    bitrateSlider.setBounds(getLocalBounds().withWidth(knobWidth).withX(knobWidth));
    postFilterSlider.setBounds(getLocalBounds().withWidth(knobWidth).withX(knobWidth * 2));
    makeupSlider.setBounds(getLocalBounds().withWidth(knobWidth).withX(knobWidth * 3));
}
