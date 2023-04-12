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
: StageWindow(p)
{
    addAndMakeVisible(squishSlider);
}

void QuantizeSection::resized()
{
    squishSlider.setBounds(getLocalBounds());
}
