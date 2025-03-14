//
// Created by arden on 8/18/23.
//

#include "TiltGraph.h"
#include "../parameterIds.h"

TiltGraph::TiltGraph(juce::AudioProcessorValueTreeState& p) : parameters(p)
{
    parameters.addParameterListener(MDCT_STEP_PARAM_ID, this);
    parameters.addParameterListener(MDCT_WINDOW_INCREMENT_PARAM_ID, this);
    tiltAmount = ((juce::AudioParameterInt*) parameters.getParameter(MDCT_WINDOW_INCREMENT_PARAM_ID))->get() / 64.f;
    shapeWidth = ((juce::AudioParameterInt*) parameters.getParameter(MDCT_STEP_PARAM_ID))->get() / 18.f;
}

TiltGraph::~TiltGraph()
{
    parameters.removeParameterListener(MDCT_STEP_PARAM_ID, this);
    parameters.removeParameterListener(MDCT_WINDOW_INCREMENT_PARAM_ID, this);
}

void TiltGraph::parameterChanged (const juce::String &parameterID, float newValue)
{
    if (parameterID == MDCT_WINDOW_INCREMENT_PARAM_ID) {
        tiltAmount = newValue / 64;
    } else if (parameterID == MDCT_STEP_PARAM_ID) {
        shapeWidth = newValue / 18;
    }
    repaint();
}

void TiltGraph::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::green);
    std::array<float, 5> rowY;
    for (int i = -2; i < 3; ++i) {
        rowY[i+2] = tiltAmount * 7 * i + getHeight() / 2;
    }
    std::array<float, 5> backX;
    std::array<float, 5> frontX;
    for (int i = -2; i < 3; ++i) {
        backX[i+2] = 12 * shapeWidth * i + getWidth() / 2;
        frontX[i+2] = 17 * shapeWidth * i + getWidth() / 2;
    }
    for (int i = 0; i < 5; ++i) {
        g.drawLine(backX[i],rowY[0],frontX[i],rowY[4], 2);
    }
    int startx, endx;
    float progress;
    for (int i = 0; i < 5; ++i) {
        progress = i / 4.f;
        startx = frontX[0] * progress + backX[0] * (1 - progress);
        endx = frontX[4] * progress + backX[4] * (1 - progress);
        g.drawLine(startx, rowY[i], endx, rowY[i], 2);
    }
}