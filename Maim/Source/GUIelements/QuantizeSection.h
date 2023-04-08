/*
  ==============================================================================

    QuantizeSection.h
    Created: 8 Apr 2023 1:50:16pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "StageWindow.h"

//==============================================================================
/*
*/
class QuantizeSection  : public StageWindow
{
public:
    QuantizeSection(juce::AudioProcessorValueTreeState& p);
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (QuantizeSection)
};
