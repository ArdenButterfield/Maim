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
                                                             reassignmentSection (p, 20, 20),
    postSection(p),
    titlePanel (p),
    parameters(p)

{
    addAndMakeVisible(psychoacousticSection);
    addAndMakeVisible(miscellaneaSection);
    addAndMakeVisible(mdctGraphSection);
    addAndMakeVisible(reassignmentSection);
    addAndMakeVisible(postSection);
    addAndMakeVisible(titlePanel);
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
    int titleHeight = 100;
    
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
    auto titleBounds = psychoacousticBounds.withHeight(titleHeight);
    psychoacousticBounds = psychoacousticBounds.withTrimmedTop(titleHeight + margin);
    auto mdctGraphBounds = bigFourRightColumn
        .withTrimmedBottom(bigFourElementHeight + margin);
    auto reassignmentBounds = bigFourLeftColumn
        .withTrimmedTop(bigFourElementHeight + margin);
    auto miscellaneaBounds = bigFourRightColumn
        .withTrimmedTop(bigFourElementHeight + margin);
    
    auto postSectionBounds = activeArea
        .withTrimmedTop(activeArea.getHeight() - postSectionHeight);
    
    psychoacousticSection.setBounds(psychoacousticBounds);
    miscellaneaSection.setBounds(miscellaneaBounds);
    mdctGraphSection.setBounds(mdctGraphBounds);
    reassignmentSection.setBounds(reassignmentBounds);
    postSection.setBounds(postSectionBounds);

    titlePanel.setBounds(titleBounds);
}
