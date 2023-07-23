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
    auto inner_bounds = getLocalBounds().withTrimmedTop(10)
                            .withTrimmedLeft(10)
                            .withTrimmedRight(10)
                            .withTrimmedBottom(10);
    biasSlider.setBounds(inner_bounds.withWidth(100));
    auto graphBounds = inner_bounds
                           .withTrimmedLeft(110);
    psychoanalGraph.setBounds(graphBounds);
    shortFrameIndicator.setBounds(graphBounds.getRight() - 15, graphBounds.getY() + 5, 10, 10);
}

