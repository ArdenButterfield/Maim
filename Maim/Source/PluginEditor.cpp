/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
MaimAudioProcessorEditor::MaimAudioProcessorEditor (MaimAudioProcessor& p) :
        AudioProcessorEditor (&p),
        audioProcessor (p),
        mainArea(p.getValueTreeState()),
        dragBox (p.getValueTreeState(), "butterflystandard", "butterflycrossed")
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (900, 700);
    addAndMakeVisible(mainArea);
}

MaimAudioProcessorEditor::~MaimAudioProcessorEditor()
{
}

//==============================================================================
void MaimAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void MaimAudioProcessorEditor::resized()
{
    mainArea.setBounds(getLocalBounds());
}

MaimAudioProcessor* MaimAudioProcessorEditor::getAudioProcessor()
{
    return &audioProcessor;
}
