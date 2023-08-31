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
    auto textRect = inner_rect.withTrimmedLeft(10);
    auto gradient = juce::ColourGradient::horizontal(MaimColours::SPLASH_COLOR_DARK,
        MaimColours::SPLASH_COLOR_LIGHT,
        inner_rect);
    g.setGradientFill(gradient);
    g.fillRect(inner_rect);


    g.setColour (juce::Colours::white);
    g.setFont(title_font);
    g.drawText ("MAIM", textRect,
        juce::Justification::centredLeft, true);
    g.setFont(tooltip_font);
    g.drawText("Version 1.0.0alpha", textRect.withTrimmedBottom(10).withTrimmedLeft(2), juce::Justification::bottomLeft, true);
}

void TitlePanel::resized()
{

}