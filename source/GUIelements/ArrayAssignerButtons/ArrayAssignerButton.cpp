//
// Created by arden on 7/23/23.
//

#include "ArrayAssignerButton.h"
#include "../MaimLookAndFeel.h"

void ArrayAssignerButton::drawPattern(juce::Graphics &g, juce::Rectangle<int> bounds, const std::array<char, buttonPixelDims * buttonPixelDims>& arr) {
    for (int r = 0; r < buttonPixelDims; ++r) {
        for (int c = 0; c < buttonPixelDims; ++c) {
            if (arr[r * buttonPixelDims + c]) {
                g.fillRect(bounds.getX() + c * buttonPixelSize,
                    bounds.getY() + r * buttonPixelSize,
                    buttonPixelSize,
                    buttonPixelSize);
            }
        }
    }
}

void ArrayAssignerButton::mouseUp (const juce::MouseEvent & e) {
    randomShapeIndex = juce::Random().nextInt({0,6});
    repaint();
    juce::Button::mouseUp(e);
}

void ArrayAssignerButton::drawResetShape(juce::Graphics &g, juce::Rectangle<int> bounds) {
    drawPattern(g, bounds, resetShape);
}
void ArrayAssignerButton::drawRandomShape(juce::Graphics &g, juce::Rectangle<int> bounds) {
    drawPattern(g, bounds, randomShapes[randomShapeIndex]);
}
void ArrayAssignerButton::drawShiftUpShape(juce::Graphics &g, juce::Rectangle<int> bounds) {
    drawPattern(g, bounds, shiftUpShape);

}
void ArrayAssignerButton::drawShiftDownShape(juce::Graphics &g, juce::Rectangle<int> bounds) {
    drawPattern(g, bounds, shiftDownShape);

}


void ArrayAssignerButton::paintButton (juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    int borderPix = 2;
    auto bounds = getLocalBounds();
    g.setColour(MaimColours::BEVEL_BLACK);
    g.fillRect(bounds);
    bounds = bounds.withSizeKeepingCentre(bounds.getWidth() - borderPix * 2, bounds.getHeight() - borderPix * 2);
    g.setColour(MaimColours::BEVEL_DARK);
    g.fillRect(bounds);
    if (shouldDrawButtonAsDown) {
        bounds = bounds.withTrimmedTop(borderPix).withTrimmedLeft(borderPix);
    } else {
        bounds = bounds.withTrimmedRight(borderPix).withTrimmedBottom(borderPix);
    }
    g.setColour(MaimColours::BEVEL_LIGHT);
    g.fillRect(bounds);
    if (shouldDrawButtonAsDown) {
        bounds = bounds.withTrimmedRight(borderPix).withTrimmedBottom(borderPix);
    } else {
        bounds = bounds.withTrimmedTop(borderPix).withTrimmedLeft(borderPix);
    }

    bounds = bounds.withSizeKeepingCentre(shapeSize, shapeSize);
    if (shouldDrawButtonAsHighlighted) {
        g.setColour(MaimColours::SPLASH_COLOR_DARK);
    } else {
        g.setColour(MaimColours::BEVEL_BLACK);
    }
    switch (buttonDesigns)
    {
        case RESET:
            drawResetShape(g, bounds);
            break;
        case RANDOM:
            drawRandomShape(g, bounds);
            break;
        case SHIFT_UP:
            drawShiftUpShape(g, bounds);
            break;
        case SHIFT_DOWN:
            drawShiftDownShape(g, bounds);
            break;
    }
}