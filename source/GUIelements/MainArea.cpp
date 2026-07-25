/*
  ==============================================================================

    MainArea.cpp
    Created: 4 Apr 2023 3:19:14pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "MainArea.h"

//==============================================================================
MainArea::MainArea(MaimAudioProcessor& processor) :
    psychoacousticSection(processor.getValueTreeState()),
    miscellaneaSection (processor.getValueTreeState()),
    mdctGraphSection (processor.getValueTreeState()),
    postSection(processor.getValueTreeState()),
    presetsSection(processor),
    titlePanel (processor.getValueTreeState()),
    reassignmentSection (processor.getValueTreeState(), 20, 20),
    opusPacketLossSection(processor.getValueTreeState()),
    parameters(processor.getValueTreeState())

{
    addAndMakeVisible(psychoacousticSection);
    addAndMakeVisible(miscellaneaSection);
    addAndMakeVisible(mdctGraphSection);
    addAndMakeVisible(reassignmentSection);
    addAndMakeVisible(postSection);
    addAndMakeVisible(presetsSection);
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
    int presetsSectionHeight = 60;

    auto activeArea = getLocalBounds()
        .withTrimmedTop(margin)
        .withTrimmedLeft(margin)
        .withTrimmedRight(margin)
        .withTrimmedBottom(margin);

    auto tilesArea = activeArea.withTrimmedRight(postSectionWidth + margin);

    auto topRow = tilesArea.withHeight(230);
    auto bottomRow = tilesArea.withTrimmedTop(topRow.getHeight() + margin);

    titlePanel.setBounds(topRow.withWidth(150));

    auto encoderColumn = topRow.withTrimmedLeft(titlePanel.getWidth() + margin);

    presetsSection.setBounds(juce::Rectangle<int>(
        encoderColumn.getX(),
        activeArea.getY(),
        activeArea.getRight() - encoderColumn.getX(),
        presetsSectionHeight));

    psychoacousticSection.setBounds(encoderColumn.withTrimmedTop(presetsSectionHeight + margin));

    postSection.setBounds(activeArea.withWidth(postSectionWidth)
                               .withRightX(activeArea.getRight())
                               .withTrimmedTop(presetsSectionHeight + margin));

    reassignmentSection.setBounds(bottomRow.withWidth(220));
    mdctGraphSection.setBounds(bottomRow.withTrimmedLeft(reassignmentSection.getWidth() + margin).withWidth(290));
    miscellaneaSection.setBounds(bottomRow.withX(mdctGraphSection.getRight() + margin).withRight(bottomRow.getRight()));
    opusPacketLossSection.setBounds(miscellaneaSection.getBounds());
}
void MainArea::parameterChanged (const juce::String&, float newValue)
{
    bool isOpus = juce::exactlyEqual(newValue, 2.0f);
    opusPacketLossSection.setVisible(isOpus);
    miscellaneaSection.setVisible(!isOpus);

}
