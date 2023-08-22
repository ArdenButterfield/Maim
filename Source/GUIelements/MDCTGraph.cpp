//
// Created by arden on 7/22/23.
//

#include "MDCTGraph.h"
#include "MaimLookAndFeel.h"

MDCTGraph::MDCTGraph(juce::AudioProcessorValueTreeState& p):
                                                                           parameters(p),
                                                                           preLine(0, 1, numSamples, MaimLookAndFeel().CONTRAST_COLOR_LIGHT, 3),
                                                                           postLine(0, 1, numSamples, MaimLookAndFeel().SPLASH_COLOR_DARK, 3)
{
    parameters.state.addListener(this);
    preSamples.fill(0);
    postSamples.fill(0);
    preLine.loadData(preSamples.data());
    postLine.loadData(postSamples.data());
/*
    parameters.addParameterListener("mdctfeedback", this);
    parameters.addParameterListener("mdctposthshift", this);
    parameters.addParameterListener("mdctpostvshift", this);
*/


    addAndMakeVisible(preLine);
    addAndMakeVisible(postLine);
}

MDCTGraph::~MDCTGraph()
{
    parameters.state.removeListener(this);
/*
    parameters.removeParameterListener("mdctfeedback", this);
    parameters.removeParameterListener("mdctposthshift", this);
    parameters.removeParameterListener("mdctpostvshift", this);
*/
}

void MDCTGraph::valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
    const juce::Identifier &property)
{
    if (property == juce::Identifier("pre")) {
        juce::Array<juce::var>* e = treeWhosePropertyHasChanged[property].getArray();
        for (int i = 0; i < numSamples; ++i) {
            preSamples[i] = (*e)[i].operator double();
        }
    } else if (property == juce::Identifier("post")) {
        juce::Array<juce::var>* t = treeWhosePropertyHasChanged[property].getArray();
        for (int i = 0; i < numSamples; ++i) {
            postSamples[i] = (*t)[i].operator double();
        }
    }

    preLine.loadData(preSamples.data());
    postLine.loadData(postSamples.data());
}

void MDCTGraph::paint (juce::Graphics& g)
{
    g.fillAll (MaimLookAndFeel().BEVEL_LIGHT);
    g.drawRect(getLocalBounds());

}

void MDCTGraph::resized()
{
    preLine.setBounds(getLocalBounds());
    postLine.setBounds(getLocalBounds());
}