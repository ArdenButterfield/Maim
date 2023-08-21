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
    sectionName.setColour(sectionName.textColourId, MaimLookAndFeel().BEVEL_BLACK);
    sectionName.setFont(sectionNameFont);
    sectionName.setText("Miscellanea", juce::dontSendNotification);
    sectionName.setJustificationType(juce::Justification::topRight);
    lameOnlyLabel.setColour(lameOnlyLabel.textColourId, MaimLookAndFeel().BEVEL_BLACK);
    lameOnlyLabel.setFont(lameLabelFont);
    lameOnlyLabel.setText("Sliders for Lame\nencoder only", juce::dontSendNotification);
    lameOnlyLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mdctBandStepSlider);
    addAndMakeVisible(mdctWindowIncrementSlider);
    addAndMakeVisible(butterflyDragBox);
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
    g.setColour(MaimLookAndFeel().BEVEL_LIGHT);
    g.fillRect(lameOnlySection);
    g.setColour(MaimLookAndFeel().BEVEL_DARK);
    g.drawRect(lameOnlySection, 1);
}

void MiscellaneaSection::resized()
{
    auto mainPart = getLocalBounds().withSizeKeepingCentre(getWidth() - 20, getHeight() - 20);
    butterflyDragBox.setBounds(mainPart.withWidth(190).withRightX(mainPart.getRight()));
    auto leftPart = mainPart.withTrimmedRight(200);
    const int titleHeight = 25;
    sectionName.setBounds(leftPart.withHeight(titleHeight));
    leftPart = leftPart.withTrimmedTop(titleHeight);
    lameOnlySection = leftPart
                          .withTrimmedTop(10);
    auto lameOnlySectionInner = lameOnlySection
                                    .withTrimmedTop(5)
                                    .withTrimmedLeft(5)
                                    .withTrimmedRight(5)
                                    .withTrimmedBottom(5);
    lameOnlyLabel.setBounds(lameOnlySectionInner);
    int halfWidth = lameOnlySectionInner.getWidth() / 2;
    tiltGraph.setBounds(lameOnlySectionInner.withHeight(63).withBottomY(lameOnlySectionInner.getBottom()));
    leftPart = lameOnlySectionInner.withTrimmedBottom(tiltGraph.getHeight());
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
