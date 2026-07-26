/*
  ==============================================================================

    StageWindow.cpp
    Created: 4 Apr 2023 3:18:09pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "StageWindow.h"

#include "../parameterIds.h"

//==============================================================================
void StageWindow::setUsableBounds()
{
    usable_bounds = getLocalBounds()
                        .withTrimmedBottom(LINE_WIDTH * 2)
                        .withTrimmedRight(LINE_WIDTH * 2)
                        .withTrimmedTop(LINE_WIDTH * 2)
                        .withTrimmedLeft(LINE_WIDTH * 2);
}

juce::Rectangle<int> StageWindow::draw_beveled_rectangle(juce::Graphics& g, juce::Rectangle<int> rect, bool raised)
{
    // Returns the rectangle inside of the bevel.

    juce::Colour top_outer, top_inner, bottom_outer, bottom_inner;
    if (raised) {
        top_outer = BEVEL_WHITE;
        top_inner = BEVEL_LIGHT;
        bottom_inner = BEVEL_DARK;
        bottom_outer = BEVEL_BLACK;
    } else {
        top_outer = BEVEL_BLACK;
        top_inner = BEVEL_DARK;
        bottom_inner = BEVEL_LIGHT;
        bottom_outer = BEVEL_WHITE;
    }

    g.setColour(bottom_outer);
    g.fillRect(rect);

    rect = rect.withTrimmedBottom(LINE_WIDTH).withTrimmedRight(LINE_WIDTH);
    g.setColour(top_outer);
    g.fillRect(rect);

    rect = rect.withTrimmedTop(LINE_WIDTH).withTrimmedLeft(LINE_WIDTH);
    g.setColour(bottom_inner);
    g.fillRect(rect);

    rect = rect.withTrimmedBottom(LINE_WIDTH).withTrimmedRight(LINE_WIDTH);
    g.setColour(top_inner);
    g.fillRect(rect);

    rect = rect.withTrimmedTop(LINE_WIDTH).withTrimmedLeft(LINE_WIDTH);
    g.setColour(PANEL_BACKGROUND_COLOR);
    g.fillRect(rect);

    return rect;
}

void StageWindow::prepare_background(juce::Graphics& g)
{
    draw_beveled_rectangle(g, getLocalBounds(), false);
}

void StageWindow::paint (juce::Graphics& g)
{
    prepare_background(g);
}

void StageWindow::timerCallback()
{
    auto p = parameters.getParameter(ENCODER_PARAM_ID)->getValue();
    if (p < 0.3) // blade
    {
        themeDarkColour = MaimColours::CONTRAST_COLOR_DARK;
        themeLightColour = MaimColours::CONTRAST_COLOR_LIGHT.withLightness (0.9f);
        contrastLightColour = MaimColours::SPLASH_COLOR_LIGHT;
        contrastDarkColour = MaimColours::SPLASH_COLOR_DARK;
    } else if (p < 0.6) // lame
    {
        themeDarkColour = MaimColours::SPLASH_COLOR_DARK;
        themeLightColour = MaimColours::SPLASH_COLOR_LIGHT;
        contrastLightColour = MaimColours::CONTRAST_COLOR_LIGHT;
        contrastDarkColour = MaimColours::CONTRAST_COLOR_DARK;
    } else // opus
    {
        themeDarkColour = MaimColours::OPUS_COLOR_DARK;
        themeLightColour = MaimColours::OPUS_COLOR_LIGHT;
        contrastLightColour = MaimColours::CONTRAST_COLOR_LIGHT;
        contrastDarkColour = MaimColours::BEVEL_WHITE;
    }
    repaint();
}
