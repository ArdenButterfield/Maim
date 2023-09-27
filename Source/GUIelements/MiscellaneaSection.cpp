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
      mdctDragBox(p, "mdctstep", "mdctwindowincr")
{
    sectionName.setColour(sectionName.textColourId, MaimColours::BEVEL_BLACK);
    sectionName.setFont(sectionNameFont);
    sectionName.setText("Miscellanea", juce::dontSendNotification);
    sectionName.setJustificationType(juce::Justification::centred);
    lameOnlyLabel.setColour(lameOnlyLabel.textColourId, MaimColours::BEVEL_BLACK);
    lameOnlyLabel.setFont(lameLabelFont);
    lameOnlyLabel.setText("Slider for Lame\nencoder only", juce::dontSendNotification);
    lameOnlyLabel.setJustificationType(juce::Justification::centred);
    butterflyLabel.setColour(sectionName.textColourId, MaimColours::BEVEL_BLACK);
    butterflyLabel.setFont(sectionNameFont.withHeight(13));
    butterflyLabel.setText("Butterfly", juce::dontSendNotification);
    butterflyLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(butterflyDragBox);
    addAndMakeVisible(mdctDragBox);
    addAndMakeVisible(butterflyLabel);
    addAndMakeVisible(sectionName);
    addAndMakeVisible(lameOnlyLabel);
    parameters.addParameterListener("encoder", this);
    bool isLame = (((juce::AudioParameterChoice*)parameters.getParameter("encoder"))->getIndex() == 1);
    mdctDragBox.setVisible(isLame);
    lameOnlyLabel.setVisible(!isLame);
}

MiscellaneaSection::~MiscellaneaSection()
{
    parameters.removeParameterListener("encoder",this);
}

void MiscellaneaSection::mouseMove (const juce::MouseEvent& event)
{

}

void MiscellaneaSection::mouseUp (const juce::MouseEvent& event)
{

}

void MiscellaneaSection::paint (juce::Graphics& g)
{
    StageWindow::paint(g);
/*
    g.setColour(MaimColours::BEVEL_LIGHT);
    g.fillRect(lameOnlySection);
    g.setColour(MaimColours::BEVEL_DARK);
    g.drawRect(lameOnlySection, 1);
*/
}

void MiscellaneaSection::resized()
{
    const int titleHeight = 25;

    auto mainPart = getLocalBounds().withSizeKeepingCentre(getWidth() - 20, getHeight() - 20);

    sectionName.setBounds(mainPart.withHeight(titleHeight));
    mainPart = mainPart.withTrimmedTop(titleHeight);

    auto buttterflyPart = mainPart.withHeight(mainPart.getHeight() / 2);
    auto mdctPart = mainPart.withTrimmedTop(buttterflyPart.getHeight());

    butterflyDragBox.setBounds(buttterflyPart);
    lameOnlyLabel.setBounds(mdctPart);
    mdctDragBox.setBounds(mdctPart);
}

void MiscellaneaSection::parameterChanged (const juce::String& parameterID, float newValue)
{
    bool isLame = (newValue == 1);
    mdctDragBox.setVisible(isLame);
    lameOnlyLabel.setVisible(!isLame);
}
