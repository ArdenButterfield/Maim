//
// Created by arden on 9/26/23.
//

#include "ButterflyDragBox.h"
#include "../MaimLookAndFeel.h"


void ButterflyDragBox::calculateGridLines(const float minVal,
    const float maxVal,
    const float step,
    const float outMin,
    const float outMax,
    std::vector<int>* v)
{
    v->clear();
    auto minGridlineIndex = ceil(minVal / step);
    for (auto i = minGridlineIndex; i * step < maxVal; ++i) {
        v->push_back(rescaleRange(i, minVal, maxVal, outMin, outMax));
    }
}

void ButterflyDragBox::drawGridlines(juce::Graphics& g)
{
    g.setColour(MaimColours::BEVEL_DARK);
    for (const auto x: verticalGridlines) {
        g.drawVerticalLine(x, box.getY(), box.getBottom());
    }
    for (const auto y: horizontalGridlines) {
        g.drawHorizontalLine(y, box.getX(), box.getRight());
    }
}

void ButterflyDragBox::drawGradients(juce::Graphics& g)
{
    const auto numGradients = 10;
    const auto gradientStep = (box.getRight() - activeZone.getX()) / numGradients;

    const juce::Colour verticalColour = MaimColours::SPLASH_COLOR_DARK.withAlpha(0.2f);
    const juce::Colour horizontalColour = MaimColours::CONTRAST_COLOR_DARK.withAlpha(0.2f);

    for (auto i = 0; i < numGradients; ++i) {
        auto barWidth = gradientStep * i * 0.7 / (numGradients);
        g.setColour(horizontalColour);
        g.fillRect(box.getX(), activeZone.getY() + i * gradientStep, box.getWidth(), barWidth);
        g.setColour(verticalColour);
        g.fillRect(activeZone.getX() + i * gradientStep, box.getY(), barWidth, box.getHeight());
    }

}

juce::Colour ButterflyDragBox::overlayFilm(const juce::Colour light, const juce::Colour film) {
    auto r = film.getFloatRed();
    auto g = film.getFloatGreen();
    auto b = film.getFloatBlue();
    auto a = film.getFloatAlpha();
    auto lr = light.getFloatRed();
    auto lg = light.getFloatGreen();
    auto lb = light.getFloatBlue();

    return juce::Colour::fromFloatRGBA(
        lr * r * a + lr * (1.f - a),
        lg * g * a + lg * (1.f - a),
        lb * b * a + lb * (1.f - a),
        light.getFloatAlpha()
    );
}

void ButterflyDragBox::drawBackground(juce::Graphics& g, int x, int y)
{
    drawGradients(g);
    drawGridlines(g);
}

juce::Colour ButterflyDragBox::getThumbFillColour(int x, int y)
{
    auto fillColour = MaimColours::BEVEL_LIGHT;
    float amountX = (xSlider->getValue() - xSlider->getMinimum()) / (xSlider->getMaximum() - xSlider->getMinimum());
    float amountY = (ySlider->getValue() - ySlider->getMinimum()) / (ySlider->getMaximum() - ySlider->getMinimum());
    fillColour = overlayFilm(fillColour, MaimColours::SPLASH_COLOR_DARK.withAlpha(amountX));
    fillColour = overlayFilm(fillColour, MaimColours::CONTRAST_COLOR_DARK.withAlpha(amountY));
    return fillColour;
}

juce::Colour ButterflyDragBox::getOutlineColour (int x, int y)
{
    return MaimColours::BEVEL_BLACK;
}

juce::Colour ButterflyDragBox::getBackgroundColour (int x, int y)
{
    return MaimColours::BEVEL_LIGHT;
}
void ButterflyDragBox::calculationsOnResize()
{
    calculateGridLines(xSlider->getMinimum(),
        xSlider->getMaximum(),
        gridStep,
        activeZone.getX(),
        activeZone.getRight(),
        &verticalGridlines);
    calculateGridLines(ySlider->getMinimum(),
        ySlider->getMaximum(),
        gridStep,
        activeZone.getY(),
        activeZone.getBottom(),
        &horizontalGridlines);
}
