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
        dragBox (p.getValueTreeState(), "butterflystandard", "butterflycrossed")
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 300);
    addAndMakeVisible(dragBox);
    addAndMakeVisible(mdctBandStepSlider);
    addAndMakeVisible(mdctBandInvertSlider);
    addAndMakeVisible(postFilterSlider);
    addAndMakeVisible(mdctPostShiftSlider);
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
    dragBox.setBounds(10, 10, 300 - 20, getHeight() - 20);
    mdctBandStepSlider.setBounds(300, 10, 150, 140);
    mdctBandInvertSlider.setBounds(300, 150, 150, 140);
    postFilterSlider.setBounds(450, 0, 150, 150);
    mdctPostShiftSlider.setBounds(450, 150, 150, 150);
}
