//
// Created by arden on 9/26/23.
//

#ifndef MAIM_SQUISHFLIPDRAGBOX_H
#define MAIM_SQUISHFLIPDRAGBOX_H

#include "DragBox.h"

class SquishFlipDragBox : public DragBox
{
public:
    SquishFlipDragBox(juce::AudioProcessorValueTreeState& p,
        const juce::String& _xParamID,
        const juce::String& _yParamID) : DragBox(p, _xParamID, _yParamID) {
    }
private:
    void drawBackground(juce::Graphics& g, int x, int y) override;
    juce::Colour getThumbFillColour(int x, int y) override;
    juce::Colour getOutlineColour(int x, int y) override;
    juce::Colour getBackgroundColour(int x, int y) override;
    void calculationsOnResize() override;

};

#endif //MAIM_SQUISHFLIPDRAGBOX_H
