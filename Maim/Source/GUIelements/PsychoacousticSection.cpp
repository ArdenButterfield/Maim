/*
  ==============================================================================

    PsychoacousticSection.cpp
    Created: 8 Apr 2023 1:50:05pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PsychoacousticSection.h"

//==============================================================================
PsychoacousticSection::PsychoacousticSection(juce::AudioProcessorValueTreeState& p) :
    StageWindow(p),
    psychoanalGraph(p)
{
    addAndMakeVisible(biasSlider);
    addAndMakeVisible(psychoanalGraph);

}

void PsychoacousticSection::resized()
{
    biasSlider.setBounds(getLocalBounds().withWidth(100));
    psychoanalGraph.setBounds(getLocalBounds().withTrimmedLeft(100));
}
