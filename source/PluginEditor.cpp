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
        mainArea(p.getValueTreeState())
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (800, 500);
    addAndMakeVisible(mainArea);
    setLookAndFeel(&maimLookAndFeel);
}

MaimAudioProcessorEditor::~MaimAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
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
