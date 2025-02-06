//
// Created by arden on 1/30/25.
//

#ifndef MAIM_PACKETLOSSDRAGBOX_H
#define MAIM_PACKETLOSSDRAGBOX_H

#include "DragBox.h"


class PacketLossDragBox : public DragBox
{
public:
    PacketLossDragBox(juce::AudioProcessorValueTreeState& p,
        const juce::String& xParamID,
        const juce::String& yParamID);
    ~PacketLossDragBox() override;
private:
    void drawBackground(juce::Graphics& g, int x, int y) override;
    juce::Colour getThumbFillColour(int x, int y) override;
    juce::Colour getOutlineColour(int x, int y) override;
    juce::Colour getBackgroundColour(int x, int y) override;
    void calculationsOnResize() override;
    void mouseEnter(const juce::MouseEvent &event) override {
        repaint();
    }
    void mouseExit(const juce::MouseEvent &event) override {
        repaint();
    }

};

#endif //MAIM_PACKETLOSSDRAGBOX_H
