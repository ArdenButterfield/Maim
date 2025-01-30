//
// Created by arden on 1/30/25.
//

#include "PacketLossDragBox.h"
#include "../MaimLookAndFeel.h"

void PacketLossDragBox::drawBackground (juce::Graphics& g, int x, int y)
{
    DragBox::drawBackground (g, x, y);
}
juce::Colour PacketLossDragBox::getThumbFillColour (int x, int y)
{
    return MaimColours::CONTRAST_COLOR_LIGHT;
}
juce::Colour PacketLossDragBox::getOutlineColour (int x, int y)
{
    return MaimColours::CONTRAST_COLOR_DARK;
}
juce::Colour PacketLossDragBox::getBackgroundColour (int x, int y)
{
    return MaimColours::BEVEL_BLACK;
}
void PacketLossDragBox::calculationsOnResize()
{
    DragBox::calculationsOnResize();
}
