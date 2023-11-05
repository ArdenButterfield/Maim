//
// Created by arden on 7/18/23.
//

#include "TitlePanel.h"

TitlePanel::TitlePanel (juce::AudioProcessorValueTreeState& p) : StageWindow(p)
{
    manualLink.setButtonText("Manual");
    manualLink.setURL(juce::URL("https://github.com/ArdenButterfield/Maim/blob/main/Docs/Manual.md"));
    manualLink.setFont(tooltip_font, false, juce::Justification::bottomLeft);
    addAndMakeVisible(manualLink);
    manualLink.setColour(juce::HyperlinkButton::textColourId, MaimColours::BEVEL_BLACK);

}

TitlePanel::~TitlePanel()
{
}


void TitlePanel::paint (juce::Graphics& g)
{
    auto inner_rect = draw_beveled_rectangle(g, getLocalBounds(), true);
    auto textRect = inner_rect;
    auto gradient = juce::ColourGradient::vertical(
        MaimColours::SPLASH_COLOR_DARK,
        MaimColours::SPLASH_COLOR_LIGHT,
        inner_rect);
    g.setGradientFill(gradient);
    g.fillRect(inner_rect);

    g.setColour (MaimColours::CONTRAST_COLOR_LIGHT);
    g.setFont(title_font);
    g.drawText ("MAIM", textRect.withTrimmedRight(4).withTrimmedTop(4),
        juce::Justification::centredTop, true);
    g.setFont(tooltip_font);
    g.setColour(MaimColours::BEVEL_BLACK);
    g.drawText("Version 1.0.0", textRect.withTrimmedBottom(2).withTrimmedLeft(2), juce::Justification::bottomLeft, true);
}

void TitlePanel::resized()
{
    manualLink.setBounds(getLocalBounds().withTrimmedLeft(6).withHeight(13).withBottomY(getHeight() - (6 + 13)));
}

