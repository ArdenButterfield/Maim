/*
  ==============================================================================

    MainArea.cpp
    Created: 4 Apr 2023 3:19:14pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include <JuceHeader.h>
#include "MainArea.h"

//==============================================================================
MainArea::MainArea(juce::AudioProcessorValueTreeState& p) :
    mdctSection(p),
    postSection(p),
    parameters(p)
{
    addAndMakeVisible(mdctSection);
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
    mdctSection.setBounds(getLocalBounds().withTrimmedRight(100));
    postSection.setBounds(getLocalBounds().withTrimmedLeft(getWidth() - 100));
}
