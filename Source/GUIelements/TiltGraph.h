//
// Created by arden on 8/18/23.
//

#ifndef MAIM_TILTGRAPH_H
#define MAIM_TILTGRAPH_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>


class TiltGraph : public juce::Component, public juce::AudioProcessorValueTreeState::Listener
{
public:
    TiltGraph(juce::AudioProcessorValueTreeState& p);
    ~TiltGraph() override;
    void paint (juce::Graphics& g) override;
private:
    void parameterChanged (const juce::String &parameterID, float newValue) override;

    juce::AudioProcessorValueTreeState& parameters;
    float tiltAmount;
    float shapeWidth;
};

#endif //MAIM_TILTGRAPH_H
