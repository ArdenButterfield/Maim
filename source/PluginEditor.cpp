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
        mainArea(p)
{
    addAndMakeVisible(mainArea);
    setLookAndFeel(&maimLookAndFeel);

    setResizable(true, true);
    constrainer.setFixedAspectRatio((double) nativeWidth / (double) nativeHeight);
    constrainer.setSizeLimits(nativeWidth / 2, nativeHeight / 2, nativeWidth * 2, nativeHeight * 2);
    setConstrainer(&constrainer);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (nativeWidth, nativeHeight);
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
    const auto scale = (float) getWidth() / (float) nativeWidth;
    mainArea.setTransform(juce::AffineTransform::scale(scale));
    mainArea.setBounds(0, 0, nativeWidth, nativeHeight);
}

MaimAudioProcessor* MaimAudioProcessorEditor::getAudioProcessor()
{
    return &audioProcessor;
}
