/*
  ==============================================================================

    MainArea.h
    Created: 4 Apr 2023 3:19:14pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "LineGraph.h"

//==============================================================================
/*
*/
class MainArea  : public juce::Component
{
public:
    MainArea();
    ~MainArea() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    LineGraph<int> testLineGraph;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainArea)
};
