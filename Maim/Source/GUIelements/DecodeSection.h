/*
  ==============================================================================

    DecodeSection.h
    Created: 8 Apr 2023 1:50:26pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "StageWindow.h"
#include "PsychoanalGraph.h"

//==============================================================================
class DecodeSection  : public StageWindow
{
public:
    DecodeSection(juce::AudioProcessorValueTreeState& p);
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DecodeSection)
};
