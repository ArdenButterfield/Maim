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
    resetButton("reset"),
    randomButton("random"),
    upButton("up", 0.75, juce::Colours::yellow),
    downButton("down", 0.25, juce::Colours::yellow),
    pTree(p)
{
    addAndMakeVisible(resetButton);
    addAndMakeVisible(randomButton);
    addAndMakeVisible(upButton);
    addAndMakeVisible(downButton);
    
    resetButton.addListener(this);
    randomButton.addListener(this);
    upButton.addListener(this);
    downButton.addListener(this);
    
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
    
    resetButton.removeListener(this);
    randomButton.removeListener(this);
    upButton.removeListener(this);
    downButton.removeListener(this);
}

void ArrayAssigner::setValue(const int index, const int newVal)
{
    if ((index < 0) || (index >= itemVals.size())) {
        return;
    }
    itemVals[index] = juce::jmax(0, juce::jmin(newVal, steps - 1));
    (*parameters[index]) = newVal;
}

void ArrayAssigner::resetGraph()
{
    for (int i = 0; i < itemVals.size(); ++i) {
        setValue(i, i);
    }
    repaint();
}

void ArrayAssigner::randomizeGraph()
{
    for (int i = 0; i < itemVals.size(); ++i) {
        setValue(i, random.nextInt(itemVals.size()));
    }
    repaint();
}

void ArrayAssigner::shiftGraph(bool up)
{
    int inc = up ? 1 : -1;
    for (int i = 0; i < itemVals.size(); ++i) {
        setValue(i, itemVals[i] + inc);
    }
    repaint();
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
    auto mainRect = getLocalBounds().withSizeKeepingCentre(getWidth() - 50, getHeight() - 50);
    auto buttonRect = mainRect.withHeight(40);
    int buttonWidth = buttonRect.getWidth() / 4;
    resetButton.setBounds(buttonRect.withWidth(buttonWidth));
    randomButton.setBounds(buttonRect.withWidth(buttonWidth).translated(buttonWidth, 0));
    upButton.setBounds(buttonRect.withWidth(buttonWidth).translated(buttonWidth * 2, 0));
    downButton.setBounds(buttonRect.withWidth(buttonWidth).translated(buttonWidth * 3, 0));
    activeArea = mainRect.withTrimmedTop(40);
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

void ArrayAssigner::buttonClicked (juce::Button * b)
{
    if (b == &resetButton) {
        resetGraph();
    } else if (b == &randomButton) {
        randomizeGraph();
    } else if (b == &upButton) {
        shiftGraph(true);
    } else if (b == &downButton) {
        shiftGraph(false);
    }
}
void ArrayAssigner::buttonStateChanged (juce::Button * b)
{
}
