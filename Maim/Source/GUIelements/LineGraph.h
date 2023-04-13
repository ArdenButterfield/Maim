/*
  ==============================================================================

    LineGraph.h
    Created: 4 Apr 2023 3:37:14pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>

//==============================================================================
/*
*/
template <class T> class LineGraph  : public juce::Component
{
public:
    LineGraph(float y_min, float y_max, int num_points, juce::Colour c) :
        numPoints(num_points),
        ymin(y_min),
        ymax(y_max),
        lineColour(c)
    {
        xVals.resize(numPoints);
        yVals.resize(numPoints);
        
        CalculateXValues();
    }
    
    ~LineGraph() override {}
    
    void loadData(const T * const data) {
        for (int i = 0; i < numPoints; ++i) {
            yVals[i] = getHeight() - (data[i] - ymin) / (ymax - ymin) * getHeight();
        }
        repaint();
    }
    
    void CalculateXValues() {
        for (int i = 0; i < numPoints; ++i) {
            xVals[i] = getWidth() * ((float)i) / ((float)numPoints);
        }
    }

    void paint (juce::Graphics& g) override {
        auto p = juce::Path();
        p.startNewSubPath(xVals[0], yVals[0]);
        for (int i = 1; i < numPoints; ++i) {
            p.lineTo(xVals[i], yVals[i]);
        }
        g.setColour(lineColour);
        g.strokePath(p, juce::PathStrokeType(2.f));
    }
    void resized() override {
        CalculateXValues();
    }

private:
    const int numPoints;
    float ymin;
    float ymax;
    std::vector<float> xVals;
    std::vector<float> yVals;
    juce::Colour lineColour;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LineGraph)
};
