/*
  ==============================================================================

    PostSection.cpp
    Created: 7 Apr 2023 9:21:27am
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "PostSection.h"
#include "../parameterIds.h"

//==============================================================================

PostSection::PostSection(juce::AudioProcessorValueTreeState& p) :
        StageWindow(p),
        driveSlider(p, DRIVE_PARAM_ID, "Drive"),
        loSlider(p, LO_CUT_PARAM_ID, "Low"),
        hiSlider(p, HI_CUT_PARAM_ID, "High"),
        makeupSlider(p, MAKEUP_GAIN_PARAM_ID, "Makeup gain"),
        mixSlider(p, MIX_PARAM_ID,"Dry/Wet")
{
    driveSlider.slider.setTextValueSuffix(" dB");
    hiSlider.slider.setTextValueSuffix(" Hz");
    loSlider.slider.setTextValueSuffix(" Hz");
    makeupSlider.slider.setTextValueSuffix(" dB");
    mixSlider.slider.setTextValueSuffix(" %");

    for (const auto slider : sliders)
    {
        slider->slider.getProperties().set("drawGrayArc", true);
        slider->slider.setRepaintsOnMouseActivity(true);
    }
    hiSlider.slider.getProperties().set("invertGrayArc", true);

    addAndMakeVisible(driveSlider);
    addAndMakeVisible(hiSlider);
    addAndMakeVisible(loSlider);
    addAndMakeVisible(makeupSlider);
    addAndMakeVisible(mixSlider);
}

void PostSection::paint (juce::Graphics& g)
{
    StageWindow::paint(g);
    g.setColour(BEVEL_LIGHT);
    g.fillRect(usable_bounds);
    g.setColour(MaimColours::BEVEL_DARK);
    for (const auto slider : { &driveSlider, &loSlider }) {
        g.drawHorizontalLine(slider->getBottom(), usable_bounds.getX() + 10,usable_bounds.getRight() - 10);
    }
}

void PostSection::resized()
{
    setUsableBounds();

    auto numSliders = static_cast<int>(sliders.size());
    auto littleSection = usable_bounds.getHeight() / numSliders;

    for (int i = 0; i < numSliders; ++i) {
        sliders[static_cast<size_t>(i)]->setBounds(usable_bounds.withHeight(littleSection).withY(usable_bounds.getY() + i * littleSection));
    }
}
