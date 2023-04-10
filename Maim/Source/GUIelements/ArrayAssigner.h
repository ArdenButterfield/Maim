/*
  ==============================================================================

    ArrayAssigner.h
    Created: 9 Apr 2023 7:03:21pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include <cmath>

//==============================================================================
/*
*/
class ArrayAssigner  : public juce::Component
{
public:
    ArrayAssigner(float numItems, int steps=0);
    ~ArrayAssigner() override;
    
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;

    void updateChart(const juce::Point<float>& mousePosition, bool strictBounds);
    
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::Rectangle<int> activeArea;
    
    float getValY(float rawVal);
    void setValue(const int index, const float newVal);
    std::vector<float> itemVals;
    int steps;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArrayAssigner)
};
