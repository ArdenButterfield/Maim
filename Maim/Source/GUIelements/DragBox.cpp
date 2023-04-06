/*
  ==============================================================================

    DragBox.cpp
    Created: 6 Apr 2023 2:16:04pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include <JuceHeader.h>
#include "DragBox.h"

//==============================================================================
DragBox::DragBox(juce::AudioProcessorValueTreeState& p,
                 const juce::String& xID,
                 const juce::String& yID) :
        xParamID(xID),
        yParamID(yID),
        parameters(p)
        
{
    xSlider = std::make_unique<juce::Slider>();
    ySlider = std::make_unique<juce::Slider>();
    xAttachment = std::make_unique<SliderAttachment>(parameters, xParamID, *xSlider);
    yAttachment = std::make_unique<SliderAttachment>(parameters, yParamID, *ySlider);
    parameters.addParameterListener(xParamID, this);
    parameters.addParameterListener(yParamID, this);
}

DragBox::~DragBox()
{
    parameters.removeParameterListener(xParamID, this);
    parameters.removeParameterListener(yParamID, this);
}

void DragBox::paint (juce::Graphics& g)
{
    int x = (xSlider->getValue() - xSlider->getMinimum()) / (xSlider->getMaximum() - xSlider->getMinimum()) * getWidth();
    int y = (ySlider->getValue() - ySlider->getMinimum()) / (ySlider->getMaximum() - ySlider->getMinimum()) * getHeight();
    thumb.setXY(x, y);

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::magenta);
    g.drawEllipse(x - thumbDrawRadius,
                  y - thumbDrawRadius,
                  thumbDrawRadius * 2,
                  thumbDrawRadius * 2, 2);
}

void DragBox::parameterChanged (const juce::String &parameterID, float newValue)
{
    repaint();
}


void DragBox::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
