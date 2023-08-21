/*
  ==============================================================================

    PostSection.cpp
    Created: 7 Apr 2023 9:21:27am
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "PostSection.h"

//==============================================================================

PostSection::PostSection(juce::AudioProcessorValueTreeState& p) :
        StageWindow(p),
        driveSlider(p, "drive", "Drive"),
        encoderAttachment(p, "encoder", encoderButton),
        bitrateSlider(p, "bitrate", "Bitrate"),
        squishSlider(p, "bitratesquish", "Squish"),
        hiSlider(p, "hicut", "High"),
        loSlider(p, "locut", "Low"),
        makeupSlider(p, "makeupgain", "Makeup gain"),
        mixSlider(p, "mix","Dry/Wet")
{
    addAndMakeVisible(driveSlider);
    addAndMakeVisible(encoderButton);
    addAndMakeVisible(bitrateSlider);
    addAndMakeVisible(squishSlider);
    addAndMakeVisible(hiSlider);
    addAndMakeVisible(loSlider);
    addAndMakeVisible(makeupSlider);
    addAndMakeVisible(mixSlider);
}

void PostSection::paint (juce::Graphics& g)
{
    StageWindow::paint(g);
    g.setColour(BEVEL_LIGHT);
    g.fillRect(usable_bounds);
    g.setColour(MaimLookAndFeel().BEVEL_DARK);
    for (const auto& section : { driveSection, bitrateSection, filterSection }) {
        g.drawVerticalLine(section.getRight(), section.getY() + 10,section.getBottom() - 10);
    }

    g.setColour(MaimLookAndFeel().BEVEL_BLACK);
    g.setFont(sectionNameFont);
    g.drawText("EQ", filterSection.withTrimmedTop(filterSection.getHeight() - 30), juce::Justification::centredTop, true);
    // g.drawText("Codec", bitrateSection.withTrimmedTop(bitrateSection.getHeight() - 30), juce::Justification::centredTop, true);
}

void PostSection::resized()
{
    setUsableBounds();
    int used_space = 0;

    auto littleSection = usable_bounds.getWidth() / 7;

    driveSection = usable_bounds.withWidth(littleSection);
    used_space += driveSection.getWidth();

    bitrateSection = usable_bounds.withTrimmedLeft(used_space).withWidth(littleSection * 2);
    used_space += bitrateSection.getWidth();

    filterSection = usable_bounds.withTrimmedLeft(used_space).withWidth(littleSection * 2);
    used_space += filterSection.getWidth();

    makeupSection = usable_bounds.withTrimmedLeft(used_space);

    driveSlider.setBounds(driveSection);

    auto top = bitrateSection.withTrimmedBottom(60);
    auto bottom = bitrateSection.withTrimmedTop(bitrateSection.getHeight() - 60);
    bitrateSlider.setBounds(top.withWidth(top.getWidth() / 2));
    squishSlider.setBounds(top.withTrimmedLeft(top.getWidth() / 2));
    encoderButton.setBounds(bottom.withSizeKeepingCentre(bottom.getWidth() - 20, 30));

    hiSlider.setBounds(filterSection.withTrimmedLeft(filterSection.getWidth() / 2));
    loSlider.setBounds(filterSection.withWidth(filterSection.getWidth() / 2));
    makeupSlider.setBounds(makeupSection.withWidth(makeupSection.getWidth() / 2));
    mixSlider.setBounds(makeupSection.withTrimmedLeft(makeupSection.getWidth() / 2));
}
