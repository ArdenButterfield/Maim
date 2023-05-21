/*
  ==============================================================================

    StageWindow.h
    Created: 4 Apr 2023 3:18:09pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

//==============================================================================
/*
*/
class StageWindow  : public juce::Component
{
public:
    StageWindow(juce::AudioProcessorValueTreeState& p);
    ~StageWindow() override;

    void paint (juce::Graphics&) override;
protected:
    juce::AudioProcessorValueTreeState& parameters;
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StageWindow)
};
