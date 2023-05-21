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
class ArrayAssigner  :
        public juce::Component,
        public juce::Timer,
        public juce::AudioProcessorValueTreeState::Listener,
        public juce::Button::Listener
{
public:
    ArrayAssigner(juce::AudioProcessorValueTreeState& p, int numItems, int steps);
    ~ArrayAssigner() override;
    
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;

    void updateChart(const juce::Point<float>& mousePosition, bool strictBounds);
    
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::TextButton resetButton;
    juce::TextButton randomButton;
    juce::ArrowButton upButton;
    juce::ArrowButton downButton;
    
    juce::Random random;
    
    void resetGraph();
    void randomizeGraph();
    void shiftGraph(bool up);
    
    
    void buttonClicked (juce::Button *) override;
    void buttonStateChanged (juce::Button *) override;
    
    void parameterChanged (const juce::String &parameterID, float newValue) override;
    void buildItemValsFromParams();
    void timerCallback() override;
    
    juce::Rectangle<int> activeArea;
    juce::AudioProcessorValueTreeState& pTree;
    std::vector<juce::AudioParameterInt*> parameters;
    float getValScreenY(const int rawVal);
    int getValIndex(const float screenY);
    void setValue(const int index, const int newVal);
    std::vector<int> itemVals;
    
    std::atomic<bool> needsRepainting;
    int steps;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ArrayAssigner)
};
