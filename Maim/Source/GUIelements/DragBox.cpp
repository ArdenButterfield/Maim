/*
  ==============================================================================

    DragBox.cpp
    Created: 6 Apr 2023 2:16:04pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include <JuceHeader.h>
#include "DragBox.h"

float rescaleRange(const float v,
                   const float oldMin,
                   const float oldMax,
                   const float newMin,
                   const float newMax)
{    return (juce::jmax(juce::jmin(v, oldMax), oldMin) - oldMin) / (oldMax - oldMin) * (newMax - newMin) + newMin;
}

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

void DragBox::calculateGridLines(const float minVal,
                                 const float maxVal,
                                 const float step,
                                 const float outMin,
                                 const float outMax,
                                 std::vector<int>* v)
{
    v->clear();
    auto minGridlineIndex = ceil(minVal / step);
    for (auto i = minGridlineIndex; i * step < maxVal; ++i) {
        v->push_back(rescaleRange(i, minVal, maxVal, outMin, outMax));
    }
}

void DragBox::drawGridlines(juce::Graphics& g)
{
    g.setColour(juce::Colours::lightgrey);
    for (const auto x: verticalGridlines) {
        g.drawVerticalLine(x, 0, getHeight());
    }
    for (const auto y: horizontalGridlines) {
        g.drawHorizontalLine(y, 0, getWidth());
    }
}

void DragBox::paint (juce::Graphics& g)
{
    int x = rescaleRange(xSlider->getValue(),
                         xSlider->getMinimum(),
                         xSlider->getMaximum(),
                         activeZone.getX(),
                         activeZone.getRight());
    int y = rescaleRange(ySlider->getValue(),
                         ySlider->getMinimum(),
                         ySlider->getMaximum(),
                         activeZone.getY(),
                         activeZone.getBottom());
    thumb.setXY(x, y);

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    drawGridlines(g);
    
    g.setColour (juce::Colours::grey);
    g.drawRoundedRectangle(0, 0, getWidth(), getHeight(), (float)thumbDrawRadius, 1.f);
    
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
    activeZone = getLocalBounds().withSizeKeepingCentre(getWidth() - thumbDrawRadius * 2,
                                                   getHeight() - thumbDrawRadius * 2);
    
    calculateGridLines(xSlider->getMinimum(),
                       xSlider->getMaximum(),
                       gridStep,
                       activeZone.getX(),
                       activeZone.getRight(),
                       &verticalGridlines);
    calculateGridLines(ySlider->getMinimum(),
                       ySlider->getMaximum(),
                       gridStep,
                       activeZone.getY(),
                       activeZone.getBottom(),
                       &horizontalGridlines);
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
    thumbHovered = true;
    xSlider->setValue(rescaleRange(event.position.getX(), activeZone.getX(), activeZone.getRight(), xSlider->getMinimum(), xSlider->getMaximum()));
    ySlider->setValue(rescaleRange(event.position.getY(), activeZone.getY(), activeZone.getBottom(), ySlider->getMinimum(), ySlider->getMaximum()));
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
