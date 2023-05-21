/*
  ==============================================================================

    PsychoacousticSection.cpp
    Created: 8 Apr 2023 1:50:05pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "PsychoacousticSection.h"

//==============================================================================
PsychoacousticSection::PsychoacousticSection(juce::AudioProcessorValueTreeState& p) :
    StageWindow(p),
    psychoanalGraph(p)
{
    parameters.state.addListener(this);

    addAndMakeVisible(biasSlider);
    addAndMakeVisible(psychoanalGraph);
    addAndMakeVisible(shortFrameIndicator);

}

PsychoacousticSection::~PsychoacousticSection()
{
    parameters.state.removeListener(this);
}

void PsychoacousticSection::valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                                     const juce::Identifier &property)
{
    if (property == juce::Identifier("shortblockindicator")) {
        if ((bool)treeWhosePropertyHasChanged.getProperty(property)) {
            shortFrameIndicator.triggerLight();
        }
    }

}


void PsychoacousticSection::resized()
{
    biasSlider.setBounds(getLocalBounds().withWidth(100));
    psychoanalGraph.setBounds(getLocalBounds().withTrimmedLeft(100));
    shortFrameIndicator.setBounds(10, 10, 20, 20);
}
