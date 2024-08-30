/*
  ==============================================================================

    DragBox.cpp
    Created: 6 Apr 2023 2:16:04pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "DragBox.h"
#include "MaimLookAndFeel.h"

//==============================================================================
DragBox::DragBox(juce::AudioProcessorValueTreeState& p,
                 const juce::String& xID,
                 const juce::String& yID) :
        xParamID(xID),
        yParamID(yID),
        parameters(p)
        
{
    startTimerHz(30);
    xSlider = std::make_unique<juce::Slider>();
    ySlider = std::make_unique<juce::Slider>();
    xAttachment = std::make_unique<SliderAttachment>(parameters, xParamID, *xSlider);
    yAttachment = std::make_unique<SliderAttachment>(parameters, yParamID, *ySlider);
    parameters.addParameterListener(xParamID, this);
    parameters.addParameterListener(yParamID, this);
    needsRepainting = false;
}

DragBox::~DragBox()
{
    parameters.removeParameterListener(xParamID, this);
    parameters.removeParameterListener(yParamID, this);
}

float DragBox::rescaleRange(const float v,
    const float oldMin,
    const float oldMax,
    const float newMin,
    const float newMax)
{
    float v2 = juce::jmax(juce::jmin(v, oldMax), oldMin);
    return (v2 - oldMin) / (oldMax - oldMin) * (newMax - newMin) + newMin;
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

    g.setColour(getBackgroundColour(x, y));
    g.fillRoundedRectangle(box.getX(), box.getY(), box.getWidth(), box.getHeight(), (float)thumbDrawRadius);   // clear the background

    drawBackground(g, x, y);

    g.setColour (getOutlineColour(x, y));
    g.drawRoundedRectangle(box.getX(), box.getY(), box.getWidth(), box.getHeight(), (float)thumbDrawRadius, 3.f);
    
    if (thumbHovered) {
        g.setColour (getThumbFillColour(x, y));
        g.fillEllipse(x - thumbDrawRadius,
                      y - thumbDrawRadius,
                      thumbDrawRadius * 2,
                      thumbDrawRadius * 2);

    }
    
    g.setColour (getOutlineColour(x, y));
    g.drawEllipse(x - thumbDrawRadius,
                  y - thumbDrawRadius,
                  thumbDrawRadius * 2,
                  thumbDrawRadius * 2,
                  2);
    needsRepainting = false;
}

void DragBox::parameterChanged (const juce::String &parameterID, float newValue)
{
    needsRepainting = true;
}


void DragBox::resized()
{
    auto side = std::min(getWidth(), getHeight());
    box = getLocalBounds().withSizeKeepingCentre(side - 2, side - 2);
    activeZone = box.withSizeKeepingCentre(box.getWidth() - thumbDrawRadius * 2,
                                                   box.getHeight() - thumbDrawRadius * 2);

    calculationsOnResize();
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
    mouseDown(event);
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

void DragBox::mouseDown(const juce::MouseEvent &event)
{
    thumbDragged = true;
    thumbHovered = true;
    xSlider->setValue(rescaleRange(event.position.getX(), activeZone.getX(), activeZone.getRight(), xSlider->getMinimum(), xSlider->getMaximum()));
    ySlider->setValue(rescaleRange(event.position.getY(), activeZone.getY(), activeZone.getBottom(), ySlider->getMinimum(), ySlider->getMaximum()));
    repaint();

}

void DragBox::timerCallback()
{
    if (needsRepainting) {
        repaint();
    }
}
