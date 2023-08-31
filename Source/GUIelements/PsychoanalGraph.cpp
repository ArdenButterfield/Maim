/*
  ==============================================================================

    PsychoanalGraph.cpp
    Created: 12 Apr 2023 3:55:31pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "PsychoanalGraph.h"
#include "MaimLookAndFeel.h"

//==============================================================================
PsychoanalGraph::PsychoanalGraph(juce::AudioProcessorValueTreeState& p):
    parameters(p),
    threshold(0, 1, 22, MaimColours::BEVEL_DARK),
    energy(0, 1, 22, MaimColours::SPLASH_COLOR_DARK)
{
    parameters.state.addListener(this);
    
    juce::Component* c = this;
    while (c->getParentComponent()) {
        c = c->getParentComponent();
    }

    energyVals.fill(0);
    thresholdVals.fill(0);
    energy.loadData(energyVals.data());
    threshold.loadData(thresholdVals.data());


    addAndMakeVisible(energy);
    addAndMakeVisible(threshold);
    
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

PsychoanalGraph::~PsychoanalGraph()
{
    parameters.state.removeListener(this);
}

void PsychoanalGraph::valueTreePropertyChanged(juce::ValueTree &treeWhosePropertyHasChanged,
                                               const juce::Identifier &property)
{
    if (property == juce::Identifier("energy")) {
        juce::Array<juce::var>* e = treeWhosePropertyHasChanged[property].getArray();
        for (int i = 0; i < 22; ++i) {
            energyVals[i] = (float)(*e)[i].operator double();
        }
    } else if (property == juce::Identifier("threshold")) {
        juce::Array<juce::var>* t = treeWhosePropertyHasChanged[property].getArray();
        std::cout << "new threshold";
        for (int i = 0; i < 22; ++i) {
            thresholdVals[i] = (float)(*t)[i].operator double();
        }
    }
    
    energy.loadData(energyVals.data());
    threshold.loadData(thresholdVals.data());
}

void PsychoanalGraph::paint (juce::Graphics& g)
{
    g.fillAll (MaimColours::BEVEL_LIGHT);   // clear the background
    g.setColour(MaimColours::BEVEL_BLACK);
    g.drawRect(getLocalBounds(), 2);
}

void PsychoanalGraph::resized()
{
    energy.setBounds(getLocalBounds());
    threshold.setBounds(getLocalBounds());
}
