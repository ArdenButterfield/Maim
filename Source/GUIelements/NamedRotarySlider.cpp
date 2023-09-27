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
    //nameHeight = std::min(std::max(getHeight() / 4, getHeight() - getWidth()), (int)(nameFont.getHeight() * 1.5));
    nameHeight = nameFont.getHeight();
    slider.setBounds(getLocalBounds().withTrimmedBottom(nameHeight));
}

void NamedRotarySlider::paint(juce::Graphics &g)
{
    g.setFont(nameFont);
    g.drawText(displayName, getLocalBounds().withTrimmedTop(getHeight() - nameHeight), juce::Justification::centredTop, true);
/*
    g.setColour(juce::Colours::magenta);
    g.drawRect(getLocalBounds());
    g.drawRect(getLocalBounds().withTrimmedBottom(nameHeight));
*/
}
void NamedRotarySlider::setFontSize (float size)
{
    nameFont = juce::Font(MaimLookAndFeel().main_font).withHeight(size);
}
