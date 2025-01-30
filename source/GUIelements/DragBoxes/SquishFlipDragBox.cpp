//
// Created by arden on 9/26/23.
//

#include "SquishFlipDragBox.h"
#include "../MaimLookAndFeel.h"

void SquishFlipDragBox::drawBackground (juce::Graphics& g, int x, int y)
{
    auto tiltAmount = ((juce::AudioParameterInt*) parameters.getParameter("mdctwindowincr"))->get() / 64.f;
    auto shapeWidth = ((juce::AudioParameterInt*) parameters.getParameter("mdctstep"))->get() / 18.f;

    g.setColour(MaimColours::CONTRAST_COLOR_DARK);
    std::array<float, 5> rowY;
    for (int i = -2; i < 3; ++i) {
        rowY[i+2] = tiltAmount * 7 * i + getHeight() / 2;
    }
    std::array<float, 5> backX;
    std::array<float, 5> frontX;
    for (int i = -2; i < 3; ++i) {
        backX[i+2] = 12 * shapeWidth * i + getWidth() / 2;
        frontX[i+2] = 17 * shapeWidth * i + getWidth() / 2;
    }
    for (int i = 0; i < 5; ++i) {
        g.drawLine(backX[i],rowY[0],frontX[i],rowY[4], 2);
    }
    int startx, endx;
    float progress;
    for (int i = 0; i < 5; ++i) {
        progress = i / 4.f;
        startx = frontX[0] * progress + backX[0] * (1 - progress);
        endx = frontX[4] * progress + backX[4] * (1 - progress);
        g.drawLine(startx, rowY[i], endx, rowY[i], 2);
    }

}
juce::Colour SquishFlipDragBox::getThumbFillColour (int x, int y)
{
    return MaimColours::CONTRAST_COLOR_LIGHT;
}
juce::Colour SquishFlipDragBox::getOutlineColour (int x, int y)
{
    return MaimColours::CONTRAST_COLOR_DARK;
}
juce::Colour SquishFlipDragBox::getBackgroundColour (int x, int y)
{
    return MaimColours::BEVEL_BLACK;
}
void SquishFlipDragBox::calculationsOnResize()
{
}
