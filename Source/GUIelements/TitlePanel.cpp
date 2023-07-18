//
// Created by arden on 7/18/23.
//

#include "TitlePanel.h"

TitlePanel::TitlePanel (juce::AudioProcessorValueTreeState& p) : StageWindow(p) {}

TitlePanel::~TitlePanel()
{
}


void TitlePanel::paint (juce::Graphics& g)
{
    auto inner_rect = draw_beveled_rectangle(g, getLocalBounds(), true);
    auto gradient = juce::ColourGradient::horizontal(juce::Colours::darkblue,
        juce::Colours::lightblue,
        inner_rect);
    g.setGradientFill(gradient);
    g.fillRect(inner_rect);


    g.setColour (juce::Colours::white);
    g.setFont(title_font);
    g.drawText ("Maim", inner_rect,
        juce::Justification::centredLeft, true);
}

void TitlePanel::resized()
{

}