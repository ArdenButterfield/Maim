/*
  ==============================================================================

    DecodeSection.cpp
    Created: 8 Apr 2023 1:50:26pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include <JuceHeader.h>
#include "DecodeSection.h"

//==============================================================================
DecodeSection::DecodeSection(juce::AudioProcessorValueTreeState& p)
: StageWindow(p)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

void DecodeSection::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
