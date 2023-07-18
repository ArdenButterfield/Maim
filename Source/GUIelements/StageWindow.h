/*
  ==============================================================================

    StageWindow.h
    Created: 4 Apr 2023 3:18:09pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include "MaimLookAndFeel.h"
//==============================================================================
/*
*/
class StageWindow  : public juce::Component
{
public:
    StageWindow(juce::AudioProcessorValueTreeState& p) : parameters(p) {}


    const juce::Font main_font = juce::Font(MaimLookAndFeel().main_font);
    const juce::Font bold_font = juce::Font(MaimLookAndFeel().bold_font);
    const juce::Font tooltip_font = juce::Font(MaimLookAndFeel().tooltip_font);
    const juce::Font title_font = main_font.withHeight(60.f);
    const juce::Font H1_font = main_font.withHeight(21.f);
    const juce::Font H2_font = main_font.withHeight(17.f);
    const juce::Font H3_font = main_font.withHeight(14.f);

    const juce::Colour PANEL_BACKGROUND_COLOR = MaimLookAndFeel().PANEL_BACKGROUND_COLOR;
    const juce::Colour BEVEL_WHITE = MaimLookAndFeel().BEVEL_WHITE;
    const juce::Colour BEVEL_LIGHT = MaimLookAndFeel().BEVEL_LIGHT;
    const juce::Colour BEVEL_DARK = MaimLookAndFeel().BEVEL_DARK;
    const juce::Colour BEVEL_BLACK = MaimLookAndFeel().BEVEL_BLACK;

    const juce::Colour TEXT_COLOR = juce::Colours::black;
    const juce::Colour BORDER_COLOR = juce::Colours::darkblue;

    void prepare_background(juce::Graphics& g);
    juce::Rectangle<int> draw_beveled_rectangle(juce::Graphics& g, juce::Rectangle<int> rect, bool raised);

    void paint (juce::Graphics& g) override;

protected:
    void setUsableBounds();
    juce::Rectangle<int> usable_bounds;
    const int LINE_WIDTH = 2; // For bevel

    juce::AudioProcessorValueTreeState& parameters;
};
