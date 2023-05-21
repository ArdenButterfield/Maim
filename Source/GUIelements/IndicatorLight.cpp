/*
  ==============================================================================

    IndicatorLight.cpp
    Created: 14 Apr 2023 2:04:50pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include <JuceHeader.h>
#include "IndicatorLight.h"

//==============================================================================
IndicatorLight::IndicatorLight()
{
    isOn = false;
    startTimerHz(30);
}

IndicatorLight::~IndicatorLight()
{
}

void IndicatorLight::triggerLight()
{
    isOn = true;
    
}

void IndicatorLight::timerCallback()
{
    isOn = false;
    repaint();
}


void IndicatorLight::paint (juce::Graphics& g)
{
    if (isOn) {
        g.setColour(onColour);
    } else {
        g.setColour(offColour);
    }
    g.fillEllipse(0, 0, getWidth(), getHeight());
}

void IndicatorLight::resized()
{

}
