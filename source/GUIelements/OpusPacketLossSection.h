//
// Created by arden on 1/29/25.
//

#ifndef MAIM_OPUSPACKETLOSSSECTION_H
#define MAIM_OPUSPACKETLOSSSECTION_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include "StageWindow.h"
#include "DragBoxes/PacketLossDragBox.h"
#include "NamedRotarySlider.h"
class OpusPacketLossSection : public StageWindow, public juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit OpusPacketLossSection (juce::AudioProcessorValueTreeState& p);
    ~OpusPacketLossSection() override;
    void resized() override;
    void paint (juce::Graphics&) override;
private:
    void parameterChanged (const juce::String &parameterID, float newValue) override;
    const juce::Font sectionNameFont = juce::Font(MaimLookAndFeel().main_font).withHeight(20.f);
    juce::Label sectionName;
    PacketLossDragBox packetLossDragBox;
    NamedRotarySlider jitterSlider;
};

#endif //MAIM_OPUSPACKETLOSSSECTION_H
