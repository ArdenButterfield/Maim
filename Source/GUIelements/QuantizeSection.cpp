/*
  ==============================================================================

    QuantizeSection.cpp
    Created: 8 Apr 2023 1:50:16pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "QuantizeSection.h"

//==============================================================================
QuantizeSection::QuantizeSection(juce::AudioProcessorValueTreeState& p)
: StageWindow(p), mdctGraph(p)
{
    addAndMakeVisible(mdctPostPitchShiftSlider);
    addAndMakeVisible(mdctPostAmpShiftSlider);
    addAndMakeVisible(MDCTfeedbackSlider);
    addAndMakeVisible(mdctGraph);

    float alpha = 0.3f;
    mdctPostAmpShiftSlider.setAlpha(alpha);
    mdctPostPitchShiftSlider.setAlpha(alpha);
    MDCTfeedbackSlider.setAlpha(alpha);

}

void QuantizeSection::resized()
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

void QuantizeSection::mouseEnter(const juce::MouseEvent& e) {

}
void QuantizeSection::mouseExit(const juce::MouseEvent& e) {

}
