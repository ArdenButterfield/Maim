//
// Created by arden on 9/26/23.
//

#ifndef MAIM_BUTTERFLYDRAGBOX_H
#define MAIM_BUTTERFLYDRAGBOX_H

#include "DragBox.h"

class ButterflyDragBox : public DragBox
{
public:
    ButterflyDragBox(juce::AudioProcessorValueTreeState& p,
        const juce::String& xParamID,
        const juce::String& yParamID) : DragBox(p, xParamID, yParamID) {}
private:
    void drawBackground(juce::Graphics& g, int x, int y) override;
    juce::Colour getThumbFillColour(int x, int y) override;
    juce::Colour getOutlineColour(int x, int y) override;
    juce::Colour getBackgroundColour(int x, int y) override;
    void calculationsOnResize() override;

    const float gridStep = 2.0f;
    std::vector<int> horizontalGridlines;
    std::vector<int> verticalGridlines;
    void calculateGridLines(const float minVal,
        const float maxVal,
        const float step,
        const float outMin,
        const float outMax,
        std::vector<int>* v);
    static juce::Colour overlayFilm(const juce::Colour light, const juce::Colour film);

    void drawGradients(juce::Graphics& g);
    void drawGridlines(juce::Graphics&);
};

#endif //MAIM_BUTTERFLYDRAGBOX_H
