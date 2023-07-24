/*
  ==============================================================================

    ArrayAssigner.cpp
    Created: 9 Apr 2023 7:03:21pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include <cmath>

#include "ArrayAssigner.h"

//==============================================================================
ArrayAssigner::ArrayAssigner(juce::AudioProcessorValueTreeState& p, int numItems, int s) :
    StageWindow(p),
    resetButton("reset", RESET),
    randomButton("random", RANDOM),
    upButton("up", SHIFT_UP),
    downButton("down", SHIFT_DOWN),
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
    if (!activeArea.contains((int)x,(int)y)) {
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
    setValue(i, getValIndex((y)));
    
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
    return activeArea.getHeight() * (1 - ((float)rawVal / (steps))) + activeArea.getY();
}

int ArrayAssigner::getValIndex(const float screenY)
{
    float v = 1 - ((screenY - activeArea.getY()) / activeArea.getHeight());
    return (int) std::round(v * (steps));
}

void ArrayAssigner::paint (juce::Graphics& g)
{
    StageWindow::paint(g);
    g.setColour(MaimLookAndFeel().BEVEL_LIGHT);
    g.fillRect(activeAreaBorder);
    g.setColour(MaimLookAndFeel().BEVEL_BLACK);
    g.drawRect(activeAreaBorder, 2);
    float singleLineWidth = (float)activeArea.getWidth() / itemVals.size();
    g.setColour(MaimLookAndFeel().PANEL_BACKGROUND_COLOR);
    for (int row = 0; row < steps + 1; ++row) {
        g.drawHorizontalLine(getValScreenY(row), activeArea.getX(), activeArea.getRight());
    }
    for (int col = 0; col < itemVals.size() + 1; ++col) {
        g.drawVerticalLine(activeArea.getX() + singleLineWidth * col, activeArea.getY(), activeArea.getBottom());
    }
    g.setColour(MaimLookAndFeel().PANEL_BACKGROUND_COLOR);
    for (int i = 0; i < itemVals.size(); ++i) {
        int left = activeArea.getX() + singleLineWidth * i;
        int right = activeArea.getX() + singleLineWidth * (i + 1);
        int top = getValScreenY(i + 1);
        int bottom = getValScreenY(i);
        g.fillRect(left, top, right - left, bottom - top);
    }

    g.setColour(MaimLookAndFeel().SPLASH_COLOR_DARK.withAlpha(0.7f));
    for (int i = 0; i < itemVals.size(); ++i) {
        int left = activeArea.getX() + singleLineWidth * i;
        int right = activeArea.getX() + singleLineWidth * (i + 1);
        int top = getValScreenY(itemVals[i] + 1);
        int bottom = getValScreenY(itemVals[i]);
        g.fillRect(left, top, right - left, bottom - top);
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
    activeAreaBorder = mainRect.withTrimmedTop(40);
    activeArea = activeAreaBorder
                     .withTrimmedTop(10)
                     .withTrimmedLeft(10)
                     .withTrimmedRight(10)
                     .withTrimmedBottom(10);
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
