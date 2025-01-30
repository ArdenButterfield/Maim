/*
  ==============================================================================

    MainArea.cpp
    Created: 4 Apr 2023 3:19:14pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "MainArea.h"

//==============================================================================
MainArea::MainArea(juce::AudioProcessorValueTreeState& p) :
    psychoacousticSection(p),
    miscellaneaSection (p),
    mdctGraphSection (p),
    postSection(p),
    titlePanel (p),
    reassignmentSection (p, 20, 20),
    opusPacketLossSection(p),
    parameters(p)

{
    addAndMakeVisible(psychoacousticSection);
    addAndMakeVisible(miscellaneaSection);
    addAndMakeVisible(mdctGraphSection);
    addAndMakeVisible(reassignmentSection);
    addAndMakeVisible(postSection);
    addAndMakeVisible(titlePanel);
    addAndMakeVisible(opusPacketLossSection);

    parameters.addParameterListener(ENCODER_PARAM_ID, this);

    bool isOpus = (((juce::AudioParameterChoice*)parameters.getParameter(ENCODER_PARAM_ID))->getIndex() == 2);
    opusPacketLossSection.setVisible(isOpus);
    miscellaneaSection.setVisible(!isOpus);

}

MainArea::~MainArea()
{
    parameters.removeParameterListener(ENCODER_PARAM_ID, this);

}

void MainArea::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
}

void MainArea::resized()
{
    int margin = 10;
    int postSectionWidth = 100;
    int titleHeight = 100;
    
    auto activeArea = getLocalBounds()
        .withTrimmedTop(margin)
        .withTrimmedLeft(margin)
        .withTrimmedRight(margin)
        .withTrimmedBottom(margin);

    auto tilesArea = activeArea.withTrimmedRight(postSectionWidth + margin);
    postSection.setBounds(activeArea.withWidth(postSectionWidth).withRightX(activeArea.getRight()));

    auto topRow = tilesArea.withHeight(230);
    auto bottomRow = tilesArea.withTrimmedTop(topRow.getHeight() + margin);

    titlePanel.setBounds(topRow.withWidth(150));
    psychoacousticSection.setBounds(topRow.withTrimmedLeft(titlePanel.getWidth() + margin));

    reassignmentSection.setBounds(bottomRow.withWidth(220));
    mdctGraphSection.setBounds(bottomRow.withTrimmedLeft(reassignmentSection.getWidth() + margin).withWidth(290));
    miscellaneaSection.setBounds(bottomRow.withX(mdctGraphSection.getRight() + margin).withRight(bottomRow.getRight()));
    opusPacketLossSection.setBounds(miscellaneaSection.getBounds());
}
void MainArea::parameterChanged (const juce::String& parameterID, float newValue)
{
    bool isOpus = (newValue == 2);
    opusPacketLossSection.setVisible(isOpus);
    miscellaneaSection.setVisible(!isOpus);

}
