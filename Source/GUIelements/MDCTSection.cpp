/*
  ==============================================================================

    MDCTSection.cpp
    Created: 7 Apr 2023 9:21:14am
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "MDCTSection.h"


MDCTSection::MDCTSection(juce::AudioProcessorValueTreeState& p)
        : StageWindow(p),
            butterflyDragBox(p, "butterflystandard", "butterflycrossed"),
      mdctBandStepSlider(p, "mdctstep", "!?"),
      mdctWindowIncrementSlider(p, "mdctwindowincr", "???")
{
    sectionName.setColour(sectionName.textColourId, MaimLookAndFeel().BEVEL_BLACK);
    sectionName.setFont(sectionNameFont);
    sectionName.setText("Miscellanea", juce::dontSendNotification);
    sectionName.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(mdctBandStepSlider);
    addAndMakeVisible(mdctWindowIncrementSlider);
    addAndMakeVisible(butterflyDragBox);
    addAndMakeVisible(sectionName);
}

void MDCTSection::resized()
{
    auto mainPart = getLocalBounds().withSizeKeepingCentre(getWidth() - 20, getHeight() - 20);
    butterflyDragBox.setBounds(mainPart.withWidth(190).withRightX(mainPart.getRight()));
    auto leftPart = mainPart.withTrimmedRight(200);
    const int titleHeight = 60;
    sectionName.setBounds(leftPart.withHeight(titleHeight));
    leftPart = leftPart.withTrimmedTop(titleHeight);


    int halfWidth = leftPart.getWidth() / 2;
    mdctBandStepSlider.setBounds(leftPart.withTrimmedLeft(halfWidth));
    mdctWindowIncrementSlider.setBounds(leftPart.withTrimmedRight(halfWidth));
}