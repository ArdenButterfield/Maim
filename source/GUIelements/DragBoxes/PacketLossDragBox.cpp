//
// Created by arden on 1/30/25.
//

#include "PacketLossDragBox.h"
#include "../MaimLookAndFeel.h"
#include "../../parameterIds.h"

PacketLossDragBox::PacketLossDragBox (juce::AudioProcessorValueTreeState& p, const juce::String& xParamID, const juce::String& yParamID) : DragBox(p, xParamID, yParamID) {
    parameters.addParameterListener(PACKET_LOSS_JITTER_PARAM_ID, this);
}

PacketLossDragBox::~PacketLossDragBox()
{
    parameters.removeParameterListener(PACKET_LOSS_JITTER_PARAM_ID, this);
}

void PacketLossDragBox::drawBackground (juce::Graphics& g, int x, int y)
{
    if (isMouseOverOrDragging()) {
        g.setColour(MaimColours::CONTRAST_COLOR_DARK.withAlpha(0.7f));
    } else {
        g.setColour(MaimColours::CONTRAST_COLOR_DARK);
    }
    auto jitter = parameters.getParameter(PACKET_LOSS_JITTER_PARAM_ID)->getValue();
    auto radius = rescaleRange(xSlider->getValue(),
        xSlider->getMinimum(),
        xSlider->getMaximum(),
        25,
        35);

    auto circleBounds = juce::Rectangle<float>()
        .withWidth(radius * 2).withHeight(radius * 2)
        .withCentre({getWidth() * 0.5f, getHeight() * 0.5f});
    g.drawEllipse(circleBounds, 2);
    auto pieSlice = juce::Path();
    auto rads = rescaleRange(ySlider->getValue(),
        ySlider->getMinimum(),
        ySlider->getMaximum(),
        0,
        juce::MathConstants<float>::twoPi);
    auto lowRads = rads - (jitter * rads);
    auto highRads = std::min(juce::MathConstants<float>::twoPi, rads + (jitter * rads));
    pieSlice.addPieSegment(circleBounds, 0, lowRads, 0);
    g.fillPath(pieSlice);

    if (jitter > 0) {
        for (float r = radius - 5; r > 0; r -= 5) {
            auto arc = juce::Path();
            arc.addCentredArc(circleBounds.getCentreX(), circleBounds.getCentreY(), r, r, 0, lowRads, highRads, true);
            g.strokePath(arc, juce::PathStrokeType(2));
        }
    }
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
