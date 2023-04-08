/*
  ==============================================================================

    PsychoacousticSection.h
    Created: 8 Apr 2023 1:50:05pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "StageWindow.h"

//==============================================================================
/*
*/
class PsychoacousticSection  : public StageWindow
{
public:
    PsychoacousticSection(juce::AudioProcessorValueTreeState& p);
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PsychoacousticSection)
};
