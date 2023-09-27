/*
  ==============================================================================

    EncoderBitrateSection.cpp
    Created: 8 Apr 2023 1:50:05pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "EncoderBitrateSection.h"

//==============================================================================
EncoderBitrateSection::EncoderBitrateSection (juce::AudioProcessorValueTreeState& p)
    : StageWindow(p),
      psychoanalGraph(p),
      biasSlider(p, "thresholdbias", "Tilt"),
      encoderAttachment(p, "encoder", encoderButton),
      bitrateSlider(p, "bitrate", "Bitrate"),
      squishSlider(p, "turbo", "Turbo")
{
    bitrateSlider.setFontSize(20.f);
    parameters.state.addListener(this);

    addAndMakeVisible(biasSlider);
    addAndMakeVisible(psychoanalGraph);
    addAndMakeVisible(encoderButton);
    addAndMakeVisible(bitrateSlider);
    addAndMakeVisible(squishSlider);
}

EncoderBitrateSection::~EncoderBitrateSection()
{
    parameters.state.removeListener(this);
}

void EncoderBitrateSection::valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                                     const juce::Identifier &property)
{
    if (property == juce::Identifier("shortblockindicator")) {
        if ((bool)treeWhosePropertyHasChanged.getProperty(property)) {
            shortFrameIndicator.triggerLight();
        }
    }

}


void EncoderBitrateSection::resized()
{
    setUsableBounds();
    const auto bigKnobWidth = usable_bounds.getWidth() * 0.45;

    bitrateSlider.setBounds(usable_bounds.withSizeKeepingCentre(bigKnobWidth, usable_bounds.getHeight()));
    const auto leftPanel = usable_bounds.withRight(bitrateSlider.getX()).withTrimmedTop(10);
    const auto rightPanel = usable_bounds.withLeft(bitrateSlider.getRight()).withTrimmedTop(10);

    encoderButton.setBounds(leftPanel.withHeight(leftPanel.getHeight() * 0.4).withTrimmedLeft(10).withTrimmedRight(10));
    squishSlider.setBounds(leftPanel.withTop(encoderButton.getBottom() + 10));

    psychoanalGraph.setBounds(rightPanel.withHeight(leftPanel.getHeight() * 0.4).withTrimmedLeft(10).withTrimmedRight(10));
    biasSlider.setBounds(rightPanel.withTop(psychoanalGraph.getBottom() + 10));
}
