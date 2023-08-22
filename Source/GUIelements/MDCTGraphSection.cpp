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
    sectionName.setColour(sectionName.textColourId, MaimLookAndFeel().BEVEL_BLACK);
    sectionName.setFont(sectionNameFont);
    sectionName.setText("Spectral Effects", juce::dontSendNotification);
    sectionName.setJustificationType(juce::Justification::centredTop);

    feedbackName.setColour(feedbackName.textColourId, MaimLookAndFeel().BEVEL_BLACK);
    feedbackName.setFont(sectionNameFont.withHeight(15));
    feedbackName.setText("Feedback", juce::dontSendNotification);
    feedbackName.setJustificationType(juce::Justification::centredTop);

    addAndMakeVisible(mdctGraph);
    addAndMakeVisible(mdctPostPitchShiftSlider);
    addAndMakeVisible(mdctPostAmpShiftSlider);
    addAndMakeVisible(MDCTfeedbackSlider);
    addAndMakeVisible(sectionName);
    addAndMakeVisible(feedbackName);
    float alpha = 0.3f;
    mdctPostAmpShiftSlider.setAlpha(alpha);
    mdctPostPitchShiftSlider.setAlpha(alpha);
    MDCTfeedbackSlider.setAlpha(alpha);
    feedbackName.setAlpha(alpha);

    MDCTfeedbackSlider.setLabel(&feedbackName);
}

void MDCTGraphSection::resized()
{
    auto graphArea = getLocalBounds()
                         .withTrimmedTop(10)
                         .withTrimmedLeft(10)
                         .withTrimmedBottom(10)
                         .withTrimmedRight(10);
    sectionName.setBounds(graphArea.withHeight(25));
    graphArea = graphArea.withTrimmedTop( 25);

    mdctGraph.setBounds(graphArea);
    auto feedbackSliderBounds =getLocalBounds().withWidth(70).withHeight(80).withRightX(graphArea.getRight()).withY(graphArea.getY());
    MDCTfeedbackSlider.setBounds(feedbackSliderBounds);
    feedbackName.setBounds(feedbackSliderBounds.getX(), feedbackSliderBounds.getBottom(), feedbackSliderBounds.getWidth(), 20);
    mdctPostPitchShiftSlider.setBounds(graphArea.withTop(graphArea.getBottom() - 60).withTrimmedLeft(30));
    mdctPostAmpShiftSlider.setBounds(graphArea.withRight(graphArea.getX() + 80).withTrimmedBottom(30));

}

void MDCTGraphSection::mouseEnter(const juce::MouseEvent& e) {

}
void MDCTGraphSection::mouseExit(const juce::MouseEvent& e) {

}
