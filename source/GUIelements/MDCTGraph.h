//
// Created by arden on 7/22/23.
//

#ifndef MAIM_MDCTGRAPH_H
#define MAIM_MDCTGRAPH_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include <array>

#include "BlockyLineGraph.h"

class MDCTGraph  : public juce::Component, public juce::ValueTree::Listener

{
public:
    explicit MDCTGraph(juce::AudioProcessorValueTreeState& p);
    ~MDCTGraph() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
        const juce::Identifier &property) override;
    juce::AudioProcessorValueTreeState& parameters;
    static const int numSamples = 576;
    std::array<float, numSamples> preSamples;
    std::array<float, numSamples> postSamples;

    BlockyLineGraph preLine;
    BlockyLineGraph postLine;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MDCTGraph)
};


#endif //MAIM_MDCTGRAPH_H
