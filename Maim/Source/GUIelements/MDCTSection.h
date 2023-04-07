/*
  ==============================================================================

    MDCTSection.h
    Created: 7 Apr 2023 9:21:14am
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "StageWindow.h"

//==============================================================================
/*
*/
class MDCTSection  : public StageWindow
{
public:
    MDCTSection(juce::AudioProcessorValueTreeState& p) : StageWindow(p) {}
    void resized() override;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MDCTSection)
};
