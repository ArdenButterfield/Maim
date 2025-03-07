/*
  ==============================================================================

    ReassignmentSection.h
    Created: 9 Apr 2023 7:03:21pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include <vector>
#include <cmath>

#include "StageWindow.h"

#include "ArrayAssignerButtons/ArrayAssignerButton.h"
#include "Mp3OnlyLabel.h"

//==============================================================================
/*
*/
class ReassignmentSection :
        public StageWindow,
        public juce::Timer,
        public juce::AudioProcessorValueTreeState::Listener,
        public juce::Button::Listener
{
public:
    ReassignmentSection (juce::AudioProcessorValueTreeState& p, int numItems, int steps);
    ~ReassignmentSection() override;
    
    void mouseDown(const juce::MouseEvent &event) override;
    void mouseDrag(const juce::MouseEvent &event) override;
    void mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel) override;

        void updateChart(const juce::Point<float>& mousePosition, bool strictBounds);
    
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    const juce::Font sectionNameFont = juce::Font(MaimLookAndFeel().main_font).withHeight(20.f);
    juce::Label sectionName;

    ArrayAssignerButton resetButton;
    ArrayAssignerButton randomButton;
    ArrayAssignerButton upButton;
    ArrayAssignerButton downButton;

    Mp3OnlyLabel mp3OnlyLabel;

    juce::Random random;
    
    void resetGraph();
    void randomizeGraph();
    void shiftGraph(bool up);
    
    
    void buttonClicked (juce::Button *) override;
    void buttonStateChanged (juce::Button *) override;
    
    void parameterChanged (const juce::String &parameterID, float newValue) override;
    void buildItemValsFromParams();
    void timerCallback() override;

    juce::Rectangle<int> activeAreaBorder;
    juce::Rectangle<int> activeArea;
    juce::AudioProcessorValueTreeState& pTree;
    std::vector<juce::AudioParameterInt*> parameters;
    float getValScreenY(const int rawVal);
    int getValIndex(const float screenY);
    void setValue(const int index, const int newVal);
    std::vector<int> itemVals;


    std::atomic<bool> needsRepainting;
    int steps;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReassignmentSection)
};
