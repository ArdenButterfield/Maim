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
        dragBox (p.getValueTreeState(), "butterfly_uu", "butterfly_du")
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (300, 300);
    addAndMakeVisible(dragBox);
    // addAndMakeVisible(mainArea);
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
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    dragBox.setBounds(10, 10, getWidth() - 20, getHeight() - 20);
    
}
