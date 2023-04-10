/*
  ==============================================================================

    ArrayAssigner.cpp
    Created: 9 Apr 2023 7:03:21pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ArrayAssigner.h"

//==============================================================================
ArrayAssigner::ArrayAssigner(juce::AudioProcessorValueTreeState& p, int numItems, int s) :
    pTree(p)
{
    parameters.resize(numItems);
    for (int i = 0; i < numItems; ++i) {
        std::stringstream id;
        id << "bandorder" << i;
        pTree.addParameterListener(id.str(), this);
        parameters[i] = (juce::AudioParameterInt*)pTree.getParameter(id.str());
    }
    itemVals.resize(numItems);
    
    for (int i = 0; i < numItems; ++i) {
        itemVals[i] = parameters[i]->get();
    }
    steps = s;

    startTimerHz(30);
    needsRepainting = false;
    
}

ArrayAssigner::~ArrayAssigner()
{
    for (int i = 0; i < 32; ++i) {
        std::stringstream id;
        id << "bandorder" << i;
        pTree.removeParameterListener(id.str(), this);
    }
}

void ArrayAssigner::setValue(const int index, const int newVal)
{
    std::cout << "set val " << index << " " << newVal << "\n";
    if ((index < 0) || (index >= itemVals.size())) {
        return;
    }
    itemVals[index] = juce::jmax(0, juce::jmin(newVal, steps - 1));
    (*parameters[index]) = newVal;
}

void ArrayAssigner::updateChart(const juce::Point<float>& mousePosition, bool strictBounds)
{
    float x, y;
    x = mousePosition.getX();
    y = mousePosition.getY();
    if (!activeArea.contains(x, y)) {
        if (strictBounds) {
            return;
        } else {
            x = juce::jmin(juce::jmax(x, (float)activeArea.getX()),
                           (float)activeArea.getRight());
            y = juce::jmin(juce::jmax(y, (float)activeArea.getY()),
                           (float)activeArea.getBottom());
        }
    }
    int i = (x - activeArea.getX()) * itemVals.size() / activeArea.getWidth();
    setValue(i, getValIndex(y));
    
    repaint();
}

void ArrayAssigner::mouseDown(const juce::MouseEvent& event)
{
    updateChart(event.position, true);
}

void ArrayAssigner::mouseDrag(const juce::MouseEvent& event)
{
    updateChart(event.position, false);
}


float ArrayAssigner::getValScreenY(const int rawVal)
{
    return activeArea.getHeight() * (1 - ((float)rawVal / (steps - 1))) + activeArea.getY();
}

int ArrayAssigner::getValIndex(const float screenY)
{
    float v = 1 - ((screenY - activeArea.getY()) / activeArea.getHeight());
    return (int) round(v * (steps - 1));
}

void ArrayAssigner::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(activeArea);
    float singleLineWidth = (float)activeArea.getWidth() / itemVals.size();
    g.setColour(juce::Colours::white);
    for (int i = 0; i < itemVals.size(); ++i) {
        g.drawHorizontalLine(getValScreenY(itemVals[i]),
                             activeArea.getX() + singleLineWidth * i,
                             activeArea.getX() + singleLineWidth * (i + 1));
    }
    needsRepainting = false;
}

void ArrayAssigner::resized()
{
    activeArea = getLocalBounds().withSizeKeepingCentre(getWidth() - 200, getHeight() - 200);
    // This method is where you should set the bounds of any child
    // components that your component contains..
}

void ArrayAssigner::parameterChanged (const juce::String &parameterID, float newValue)
{
    buildItemValsFromParams();
    
    needsRepainting = true;
}

void ArrayAssigner::buildItemValsFromParams()
{
    for (int i = 0; i < itemVals.size(); ++i) {
        setValue(i, parameters[i]->get());
    }
}

void ArrayAssigner::timerCallback()
{
    if (needsRepainting) {
        repaint();
    }
}
