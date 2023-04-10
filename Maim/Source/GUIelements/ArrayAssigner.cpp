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
ArrayAssigner::ArrayAssigner(float numItems, int s)
{
    itemVals.resize(numItems);
    steps = s;
    
    for (int i = 0; i < itemVals.size(); ++i) {
        setValue(i, (float)i / itemVals.size());
    }
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

ArrayAssigner::~ArrayAssigner()
{
}

void ArrayAssigner::setValue(const int index, const float newVal)
{
    if ((0 > newVal) || (newVal > 1) || (index < 0) || (index > itemVals.size())) {
        return;
    }
    if (steps) {
        itemVals[index] = round(newVal * (steps-1)) / (steps-1);
    } else {
        itemVals[index] = newVal;
    }
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
    float v = 1 - ((y - activeArea.getY()) / activeArea.getHeight());
    setValue(i, v);
    
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


float ArrayAssigner::getValY(float rawVal)
{
    return activeArea.getHeight() * (1 - rawVal) + activeArea.getY();
}

void ArrayAssigner::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    
    
    g.fillAll (juce::Colours::black);   // clear the background
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(activeArea);
    float singleLineWidth = (float)activeArea.getWidth() / itemVals.size();
    g.setColour(juce::Colours::white);
    for (int i = 0; i < itemVals.size(); ++i) {
        g.drawHorizontalLine(getValY(itemVals[i]),
                             activeArea.getX() + singleLineWidth * i,
                             activeArea.getX() + singleLineWidth * (i + 1));
    }
}

void ArrayAssigner::resized()
{
    activeArea = getLocalBounds().withSizeKeepingCentre(getWidth() - 200, getHeight() - 200);
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
