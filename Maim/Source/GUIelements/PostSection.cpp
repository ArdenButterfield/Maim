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
}

void PostSection::resized()
{
    postFilterSlider.setBounds(getLocalBounds().withTrimmedTop(getHeight()/2));
    bitrateSlider.setBounds(getLocalBounds().withTrimmedBottom(getHeight()/2));
}
