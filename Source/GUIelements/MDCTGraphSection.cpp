/*
  ==============================================================================

    MDCTGraphSection.cpp
    Created: 8 Apr 2023 1:50:16pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "MDCTGraphSection.h"

//==============================================================================
MDCTGraphSection::MDCTGraphSection (juce::AudioProcessorValueTreeState& p)
: StageWindow(p), mdctGraph(p)
{
    addAndMakeVisible(mdctGraph);
    addAndMakeVisible(mdctPostPitchShiftSlider);
    addAndMakeVisible(mdctPostAmpShiftSlider);
    addAndMakeVisible(MDCTfeedbackSlider);

    float alpha = 0.3f;
    mdctPostAmpShiftSlider.setAlpha(alpha);
    mdctPostPitchShiftSlider.setAlpha(alpha);
    MDCTfeedbackSlider.setAlpha(alpha);

}

void MDCTGraphSection::resized()
{
    auto graphArea = getLocalBounds()
                         .withTrimmedTop(10)
                         .withTrimmedLeft(10)
                         .withTrimmedBottom(10)
                         .withTrimmedRight(10);
    mdctGraph.setBounds(graphArea);
    MDCTfeedbackSlider.setBounds(getLocalBounds().withWidth(70).withHeight(80).withRightX(graphArea.getRight()).withY(graphArea.getY()));
    mdctPostPitchShiftSlider.setBounds(graphArea.withTop(graphArea.getBottom() - 60).withTrimmedLeft(30));
    mdctPostAmpShiftSlider.setBounds(graphArea.withRight(graphArea.getX() + 80).withTrimmedBottom(30));

}

void MDCTGraphSection::mouseEnter(const juce::MouseEvent& e) {

}
void MDCTGraphSection::mouseExit(const juce::MouseEvent& e) {

}
