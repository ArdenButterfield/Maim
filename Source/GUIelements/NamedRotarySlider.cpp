//
// Created by arden on 7/20/23.
//

#include "NamedRotarySlider.h"

NamedRotarySlider::NamedRotarySlider(juce::AudioProcessorValueTreeState& p,
    juce::String _parameterName,
    juce::String _displayName) :
                                   attachment(p,_parameterName,slider),
                                   displayName(_displayName)
{
    addAndMakeVisible(slider);
}

void NamedRotarySlider::resized()
{
    nameHeight = std::max(getHeight() / 4, getHeight() - getWidth());
    slider.setBounds(getLocalBounds().withTrimmedBottom(nameHeight));
}

void NamedRotarySlider::paint(juce::Graphics &g)
{
    g.setFont(nameFont);
    g.drawText(displayName, getLocalBounds().withTrimmedTop(getHeight() - nameHeight), juce::Justification::centredTop, true);
}
