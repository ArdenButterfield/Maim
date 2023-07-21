/*
  ==============================================================================

    PostSection.cpp
    Created: 7 Apr 2023 9:21:27am
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "PostSection.h"

//==============================================================================

PostSection::PostSection(juce::AudioProcessorValueTreeState& p) : StageWindow(p),
                                                                   driveSlider(p, "drive", "Drive"),
                                                                   encoderSlider(p, "encoder", "Encoder"),
                                                                   bitrateSlider(p, "bitrate", "Bitrate"),
                                                                   hiSlider(p, "hicut", "High"),
                                                                   loSlider(p, "locut", "Low"),
                                                                   makeupSlider(p, "makeupgain", "Makeup gain")
{
    addAndMakeVisible(driveSlider);
    addAndMakeVisible(encoderSlider);
    addAndMakeVisible(bitrateSlider);
    addAndMakeVisible(hiSlider);
    addAndMakeVisible(loSlider);
    addAndMakeVisible(makeupSlider);
}

void PostSection::paint (juce::Graphics& g)
{
    StageWindow::paint(g);
    g.setColour(MaimLookAndFeel().BEVEL_DARK);
    for (const auto& section : { driveSection, bitrateSection, filterSection }) {
        g.drawVerticalLine(section.getRight(), section.getY() + 10,section.getBottom() - 10);
    }

    g.setColour(MaimLookAndFeel().BEVEL_BLACK);
    g.setFont(eqFont);
    g.drawText("EQ", filterSection.withTrimmedTop(filterSection.getHeight() - 30), juce::Justification::centredTop, true);

}

void PostSection::resized()
{
    setUsableBounds();
    int used_space = 0;

    driveSection = usable_bounds.withWidth(125);
    used_space += driveSection.getWidth();

    bitrateSection = usable_bounds.withTrimmedLeft(used_space).withWidth(250);
    used_space += bitrateSection.getWidth();

    filterSection = usable_bounds.withTrimmedLeft(used_space).withWidth(250);
    used_space += filterSection.getWidth();

    makeupSection = usable_bounds.withTrimmedLeft(used_space);

    driveSlider.setBounds(driveSection);
    bitrateSlider.setBounds(bitrateSection.withWidth(bitrateSection.getWidth() / 2));
    encoderSlider.setBounds(bitrateSection.withTrimmedLeft(bitrateSection.getWidth() / 2));
    hiSlider.setBounds(filterSection.withTrimmedLeft(filterSection.getWidth() / 2));
    loSlider.setBounds(filterSection.withWidth(filterSection.getWidth() / 2));
    makeupSlider.setBounds(makeupSection);
}
