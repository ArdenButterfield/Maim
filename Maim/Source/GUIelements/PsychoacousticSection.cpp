/*
  ==============================================================================

    PsychoacousticSection.cpp
    Created: 8 Apr 2023 1:50:05pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include <JuceHeader.h>
#include "PsychoacousticSection.h"

//==============================================================================
PsychoacousticSection::PsychoacousticSection(juce::AudioProcessorValueTreeState& p)
: StageWindow(p),
arrayAssigner(p, 32, 32)
{
    addAndMakeVisible(arrayAssigner);
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

void PsychoacousticSection::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    arrayAssigner.setBounds(getLocalBounds());
}
