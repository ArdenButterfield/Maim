/*
  ==============================================================================

    MiscellaneaSection.cpp
    Created: 7 Apr 2023 9:21:14am
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "MiscellaneaSection.h"

MiscellaneaSection::MiscellaneaSection (juce::AudioProcessorValueTreeState& p)
        : StageWindow(p),
            butterflyDragBox(p, "butterflystandard", "butterflycrossed"),
      mdctBandStepSlider(p, "mdctstep", "Squish"),
      mdctWindowIncrementSlider(p, "mdctwindowincr", "Flip"),
      tiltGraph(p)
{
    sectionName.setColour(sectionName.textColourId, MaimColours::BEVEL_BLACK);
    sectionName.setFont(sectionNameFont);
    sectionName.setText("Miscellanea", juce::dontSendNotification);
    sectionName.setJustificationType(juce::Justification::centred);
    lameOnlyLabel.setColour(lameOnlyLabel.textColourId, MaimColours::BEVEL_BLACK);
    lameOnlyLabel.setFont(lameLabelFont);
    lameOnlyLabel.setText("Sliders for Lame\nencoder only", juce::dontSendNotification);
    lameOnlyLabel.setJustificationType(juce::Justification::centred);
    butterflyLabel.setColour(sectionName.textColourId, MaimColours::BEVEL_BLACK);
    butterflyLabel.setFont(sectionNameFont.withHeight(13));
    butterflyLabel.setText("Butterfly", juce::dontSendNotification);
    butterflyLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mdctBandStepSlider);
    addAndMakeVisible(mdctWindowIncrementSlider);
    addAndMakeVisible(butterflyDragBox);
    addAndMakeVisible(butterflyLabel);
    addAndMakeVisible(sectionName);
    addAndMakeVisible(tiltGraph);
    addAndMakeVisible(lameOnlyLabel);
    parameters.addParameterListener("encoder", this);
    bool isLame = (((juce::AudioParameterChoice*)parameters.getParameter("encoder"))->getIndex() == 1);
    mdctWindowIncrementSlider.setVisible(isLame);
    mdctBandStepSlider.setVisible(isLame);
    tiltGraph.setVisible(isLame);
    lameOnlyLabel.setVisible(!isLame);
}

MiscellaneaSection::~MiscellaneaSection()
{
    parameters.removeParameterListener("encoder",this);
}

void MiscellaneaSection::paint (juce::Graphics& g)
{
    StageWindow::paint(g);
    g.setColour(MaimColours::BEVEL_LIGHT);
    g.fillRect(lameOnlySection);
    g.setColour(MaimColours::BEVEL_DARK);
    g.drawRect(lameOnlySection, 1);
}

void MiscellaneaSection::resized()
{
    const int titleHeight = 25;
    const int rightPartWidth = 170;
    auto mainPart = getLocalBounds().withSizeKeepingCentre(getWidth() - 20, getHeight() - 20);
    sectionName.setBounds(mainPart.withHeight(titleHeight));
    mainPart = mainPart.withTrimmedTop(titleHeight);
    auto rightPart = mainPart.withWidth(rightPartWidth).withRightX(mainPart.getRight());
    auto leftPart = mainPart.withTrimmedRight(rightPartWidth + 10);
    butterflyDragBox.setBounds(leftPart.withHeight(leftPart.getWidth()));
    butterflyLabel.setBounds(leftPart.withTrimmedTop(leftPart.getWidth()));
    lameOnlySection = rightPart.withTrimmedTop(5);
    auto lameOnlySectionInner = lameOnlySection
                                    .withTrimmedTop(5)
                                    .withTrimmedLeft(15)
                                    .withTrimmedRight(15)
                                    .withTrimmedBottom(5);
    lameOnlyLabel.setBounds(lameOnlySectionInner);
    int halfWidth = lameOnlySectionInner.getWidth() / 2;
    tiltGraph.setBounds(lameOnlySectionInner.withHeight(57).withBottomY(lameOnlySectionInner.getBottom()));
    mdctBandStepSlider.setBounds(lameOnlySectionInner.withTrimmedLeft(halfWidth));
    mdctWindowIncrementSlider.setBounds(lameOnlySectionInner.withTrimmedRight(halfWidth));
}
void MiscellaneaSection::parameterChanged (const juce::String& parameterID, float newValue)
{
    bool isLame = (newValue == 1);
    mdctWindowIncrementSlider.setVisible(isLame);
    mdctBandStepSlider.setVisible(isLame);
    tiltGraph.setVisible(isLame);
    lameOnlyLabel.setVisible(!isLame);
}
