/*
  ==============================================================================

    StageWindow.cpp
    Created: 4 Apr 2023 3:18:09pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include <JuceHeader.h>
#include "StageWindow.h"

//==============================================================================
StageWindow::StageWindow(juce::AudioProcessorValueTreeState& p) :
    parameters(p)
{
    
}

StageWindow::~StageWindow()
{
}

void StageWindow::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (juce::Colours::grey);   // clear the background

    g.setColour (juce::Colours::lightgrey);
    g.drawRect (getLocalBounds(), 5);   // draw an outline around the component
}
