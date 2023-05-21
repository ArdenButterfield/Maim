/*
  ==============================================================================

    PostSection.cpp
    Created: 7 Apr 2023 9:21:27am
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "PostSection.h"

//==============================================================================

PostSection::PostSection(juce::AudioProcessorValueTreeState& p) : StageWindow(p) {
    addAndMakeVisible(postFilterSlider);
    addAndMakeVisible(bitrateSlider);
    addAndMakeVisible(driveSlider);
    addAndMakeVisible(makeupSlider);
    addAndMakeVisible(encoderSlider);
}

void PostSection::resized()
{
    int knobWidth = getWidth() / 5;
    driveSlider.setBounds(getLocalBounds().withWidth(knobWidth));
    bitrateSlider.setBounds(getLocalBounds().withWidth(knobWidth).withX(knobWidth));
    encoderSlider.setBounds(getLocalBounds().withWidth(knobWidth).withX(knobWidth * 2));
    postFilterSlider.setBounds(getLocalBounds().withWidth(knobWidth).withX(knobWidth * 3));
    makeupSlider.setBounds(getLocalBounds().withWidth(knobWidth).withX(knobWidth * 4));
}
