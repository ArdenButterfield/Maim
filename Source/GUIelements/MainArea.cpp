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
    auto scale = getWidth() / 800.;
    auto margin = 10.0 * scale;
    auto postSectionWidth = 100.0 * scale;

    auto activeArea = getLocalBounds()
        .withTrimmedTop((int)margin)
        .withTrimmedLeft((int)margin)
        .withTrimmedRight((int)margin)
        .withTrimmedBottom((int)margin);

    auto tilesArea = activeArea.withTrimmedRight((int)(postSectionWidth + margin));
    postSection.setBounds(activeArea.withWidth((int)postSectionWidth).withRightX(activeArea.getRight()));

    auto topRow = tilesArea.withHeight((int)(230.0 * scale));
    auto bottomRow = tilesArea.withTrimmedTop((int)(topRow.getHeight() + margin));

    titlePanel.setBounds(topRow.withWidth((int)(150.0 * scale)));
    psychoacousticSection.setBounds(topRow.withTrimmedLeft((int)(titlePanel.getWidth() + margin)));

    reassignmentSection.setBounds(bottomRow.withWidth((int)(220.0 * scale)));
    mdctGraphSection.setBounds(bottomRow.withTrimmedLeft((int)(reassignmentSection.getWidth() + margin)).withWidth((int)(290.0 * scale)));
    miscellaneaSection.setBounds(bottomRow.withX((int)(mdctGraphSection.getRight() + margin)).withRight(bottomRow.getRight()));
}
