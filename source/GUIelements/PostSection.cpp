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
        hiSlider(p, HI_CUT_PARAM_ID, "High"),
        loSlider(p, LO_CUT_PARAM_ID, "Low"),
        makeupSlider(p, MAKEUP_GAIN_PARAM_ID, "Makeup gain"),
        mixSlider(p, MIX_PARAM_ID,"Dry/Wet")
{
    driveSlider.slider.setTextValueSuffix(" dB");
    hiSlider.slider.setTextValueSuffix(" Hz");
    loSlider.slider.setTextValueSuffix(" Hz");
    makeupSlider.slider.setTextValueSuffix(" dB");
    mixSlider.slider.setTextValueSuffix(" %");

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
    int used_space = 0;

    auto littleSection = usable_bounds.getHeight() / sliders.size();

    for (int i = 0; i < sliders.size(); ++i) {
        sliders[i]->setBounds(usable_bounds.withHeight(littleSection).withY(usable_bounds.getY() + i * littleSection));
    }
}
