/*
  ==============================================================================

    EncoderBitrateSection.cpp
    Created: 8 Apr 2023 1:50:05pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "EncoderBitrateSection.h"
#include "../parameterIds.h"

//==============================================================================
EncoderBitrateSection::EncoderBitrateSection (juce::AudioProcessorValueTreeState& p)
    : StageWindow(p),
      psychoanalGraph(p),
      biasSlider(p, THRESHOLD_BIAS_PARAM_ID, "Tilt"),
      encoderAttachment(p, ENCODER_PARAM_ID, encoderSelection),
      bitrateSlider(p, BITRATE_PARAM_ID, "Bitrate"),
      squishSlider(p, TURBO_PARAM_ID, "Turbo"),
      errorSlider(p, ERROR_PARAM_ID, "Error")
{
    bitrateSlider.slider.setTextValueSuffix(" kb/s");

    bitrateSlider.setFontSize(20.f);
    parameters.state.addListener(this);

    addAndMakeVisible(biasSlider);
    addAndMakeVisible(psychoanalGraph);
//    addAndMakeVisible(encoderButton);
    addAndMakeVisible(encoderSelection);
    encoderSelection.addItem("blade", 1);
    encoderSelection.addItem("lame", 2);
    encoderSelection.addItem("opus", 3);

    addAndMakeVisible(bitrateSlider);
    addAndMakeVisible(squishSlider);
    addAndMakeVisible(errorSlider);
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
    const auto bigKnobWidth = usable_bounds.getWidth() * 0.4;

    bitrateSlider.setBounds(usable_bounds.withSizeKeepingCentre(bigKnobWidth, usable_bounds.getHeight()));
    const auto leftPanel = usable_bounds.withRight(bitrateSlider.getX()).withTrimmedTop(10);
    const auto rightPanel = usable_bounds.withLeft(bitrateSlider.getRight()).withTrimmedTop(10);

//    encoderButton.setBounds(leftPanel.withHeight(leftPanel.getHeight() * 0.4).withTrimmedLeft(10).withTrimmedRight(10));
    encoderSelection.setBounds(leftPanel.withHeight(leftPanel.getHeight() * 0.4).withTrimmedLeft(10).withTrimmedRight(10));
    auto belowEncoder = leftPanel.withTop(encoderSelection.getBottom() + 10);
    squishSlider.setBounds(belowEncoder.withWidth(belowEncoder.getWidth() / 2));
    errorSlider.setBounds(belowEncoder.withLeft(squishSlider.getRight()));

    psychoanalGraph.setBounds(rightPanel.withHeight(leftPanel.getHeight() * 0.4).withTrimmedLeft(10).withTrimmedRight(10));
    biasSlider.setBounds(rightPanel.withTop(psychoanalGraph.getBottom() + 10));
}
