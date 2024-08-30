/*
  ==============================================================================

    StageWindow.cpp
    Created: 4 Apr 2023 3:18:09pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "StageWindow.h"

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
