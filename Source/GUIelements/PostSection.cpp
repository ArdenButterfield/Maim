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

void PostSection::paint (juce::Graphics& g)
{
    StageWindow::paint(g);
    g.setColour(MaimLookAndFeel().BEVEL_DARK);
    for (const auto& section : { driveSection, bitrateSection, filterSection }) {
        g.drawVerticalLine(section.getRight(), section.getY() + 10,section.getBottom() - 10);

    }
}

void PostSection::resized()
{
    setUsableBounds();
    int used_space = 0;

    driveSection = usable_bounds.withWidth(75);
    used_space += driveSection.getWidth();

    bitrateSection = usable_bounds.withTrimmedLeft(used_space).withWidth(150);
    used_space += bitrateSection.getWidth();

    filterSection = usable_bounds.withTrimmedLeft(used_space).withWidth(75);
    used_space += filterSection.getWidth();

    makeupSection = usable_bounds.withTrimmedLeft(used_space);

    int knobWidth = getWidth() / 5;
    driveSlider.setBounds(driveSection);
    bitrateSlider.setBounds(bitrateSection.withWidth(bitrateSection.getWidth() / 2));
    encoderSlider.setBounds(bitrateSection.withTrimmedLeft(bitrateSection.getWidth() / 2));
    postFilterSlider.setBounds(filterSection);
    makeupSlider.setBounds(makeupSection);
}
