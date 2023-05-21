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
    mdctSection(p),
    quantizeSection(p),
    decodeSection(p),
    postSection(p),
    parameters(p)
{
    addAndMakeVisible(psychoacousticSection);
    addAndMakeVisible(mdctSection);
    addAndMakeVisible(quantizeSection);
    addAndMakeVisible(decodeSection);
    addAndMakeVisible(postSection);
}

MainArea::~MainArea()
{
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
    int postSectionHeight = 150;
    
    auto activeArea = getLocalBounds()
        .withTrimmedTop(margin)
        .withTrimmedLeft(margin)
        .withTrimmedRight(margin)
        .withTrimmedBottom(margin);
    
    auto bigFour = activeArea
        .withTrimmedBottom(postSectionHeight)
        .withTrimmedBottom(margin);
    
    int bigFourElementWidth = (bigFour.getWidth() - margin) / 2;
    int bigFourElementHeight = (bigFour.getHeight() - margin) / 2;
    
    auto bigFourLeftColumn = bigFour
        .withTrimmedRight(bigFourElementWidth + margin);
    auto bigFourRightColumn = bigFour
        .withTrimmedLeft(bigFourElementWidth + margin);
    
    auto psychoacousticBounds = bigFourLeftColumn
        .withTrimmedBottom(bigFourElementHeight + margin);
    auto mdctBounds = bigFourRightColumn
        .withTrimmedBottom(bigFourElementHeight + margin);
    auto quantizeBounds = bigFourLeftColumn
        .withTrimmedTop(bigFourElementHeight + margin);
    auto decodeBounds = bigFourRightColumn
        .withTrimmedTop(bigFourElementHeight + margin);
    
    auto postSectionBounds = activeArea
        .withTrimmedTop(activeArea.getHeight() - postSectionHeight);
    
    psychoacousticSection.setBounds(psychoacousticBounds);
    mdctSection.setBounds(mdctBounds);
    quantizeSection.setBounds(quantizeBounds);
    decodeSection.setBounds(decodeBounds);
    postSection.setBounds(postSectionBounds);
}
