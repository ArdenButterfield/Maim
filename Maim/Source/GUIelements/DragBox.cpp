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

    if (thumbHovered) {
        g.setColour (juce::Colours::red);
        g.fillEllipse(x - thumbDrawRadius,
                      y - thumbDrawRadius,
                      thumbDrawRadius * 2,
                      thumbDrawRadius * 2);

    }
    
    g.setColour (juce::Colours::magenta);
    g.drawEllipse(x - thumbDrawRadius,
                  y - thumbDrawRadius,
                  thumbDrawRadius * 2,
                  thumbDrawRadius * 2,
                  2);
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

void DragBox::mouseMove(const juce::MouseEvent& event)
{
    bool inRange = (thumb.getDistanceSquaredFrom(event.getPosition()) <
                    thumbSensitivityRadius * thumbSensitivityRadius);
    if (inRange and not thumbHovered) {
        thumbHovered = true;
        repaint();
    }
    if (thumbHovered and not inRange) {
        thumbHovered = false;
        repaint();
    }
}

void DragBox::mouseDrag(const juce::MouseEvent& event)
{
    thumbDragged = true;
    xSlider->setValue(((float)event.x / getWidth()) * (xSlider->getMaximum() - xSlider->getMinimum()) + xSlider->getMinimum());
    ySlider->setValue(((float)event.y / getHeight()) * (ySlider->getMaximum() - ySlider->getMinimum()) + ySlider->getMinimum());
    repaint();
}


void DragBox::mouseDoubleClick(const juce::MouseEvent& event)
{
    if (xSlider->isDoubleClickReturnEnabled()) {
        xSlider->setValue(xSlider->getDoubleClickReturnValue());
    }
    if (ySlider->isDoubleClickReturnEnabled()) {
        ySlider->setValue(ySlider->getDoubleClickReturnValue());
    }
    
}
