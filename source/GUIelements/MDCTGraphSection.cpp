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
: StageWindow(p), mdctGraph(p), parameters(p)
{
    sectionName.setColour(sectionName.textColourId, MaimColours::BEVEL_BLACK);
    sectionName.setFont(sectionNameFont);
    sectionName.setText("Spectral Effects", juce::dontSendNotification);
    sectionName.setJustificationType(juce::Justification::centred);

    feedbackName.setColour(feedbackName.textColourId, MaimColours::BEVEL_BLACK);
    feedbackName.setFont(sectionNameFont.withHeight(15));
    feedbackName.setText("Feedback", juce::dontSendNotification);
    feedbackName.setJustificationType(juce::Justification::centredTop);

    addAndMakeVisible(mdctGraph);
    addAndMakeVisible(mdctPostPitchShiftSlider);
    addAndMakeVisible(mdctPostAmpShiftSlider);
    addAndMakeVisible(MDCTfeedbackSlider);
    addAndMakeVisible(sectionName);
    addAndMakeVisible(feedbackName);
    addAndMakeVisible(mp3OnlyLabel);

    parameters.addParameterListener(ENCODER_PARAM_ID, this);
    bool isOpus = (((juce::AudioParameterChoice*)parameters.getParameter(ENCODER_PARAM_ID))->getIndex() == 2);
    mdctGraph.setVisible(!isOpus);
    mdctPostPitchShiftSlider.setVisible(!isOpus);
    mdctPostAmpShiftSlider.setVisible(!isOpus);
    MDCTfeedbackSlider.setVisible(!isOpus);
    feedbackName.setVisible(!isOpus);
    mp3OnlyLabel.setVisible(isOpus);

    float alpha = 0.3f;
    mdctPostAmpShiftSlider.setAlpha(alpha);
    mdctPostPitchShiftSlider.setAlpha(alpha);
    MDCTfeedbackSlider.setAlpha(alpha);
    feedbackName.setAlpha(alpha);

    MDCTfeedbackSlider.setLabel(&feedbackName);
    MDCTfeedbackSlider.setTextValueSuffix(" %");
}

void MDCTGraphSection::resized()
{
    auto graphArea = getLocalBounds()
                         .withTrimmedLeft(standardMargin)
                         .withTrimmedRight(standardMargin)
                         .withTrimmedBottom(standardMargin);
    sectionName.setBounds(graphArea.withHeight(headerHeight));
    graphArea = graphArea.withTrimmedTop( headerHeight);
    mp3OnlyLabel.setBounds(graphArea);

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
void MDCTGraphSection::parameterChanged (const juce::String& parameterID, float newValue)
{
    bool isOpus = (((juce::AudioParameterChoice*)parameters.getParameter(ENCODER_PARAM_ID))->getIndex() == 2);
    mdctGraph.setVisible(!isOpus);
    mdctPostPitchShiftSlider.setVisible(!isOpus);
    mdctPostAmpShiftSlider.setVisible(!isOpus);
    MDCTfeedbackSlider.setVisible(!isOpus);
    feedbackName.setVisible(!isOpus);
    mp3OnlyLabel.setVisible(isOpus);
}
