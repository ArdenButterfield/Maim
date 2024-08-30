//
// Created by arden on 7/22/23.
//

#ifndef MAIM_BLOCKYLINEGRAPH_H
#define MAIM_BLOCKYLINEGRAPH_H

#include "LineGraph.h"

class BlockyLineGraph : public LineGraph<float>
{
public:
    BlockyLineGraph(float y_min, float y_max, int num_points, juce::Colour c, int pixelSize);
    ~BlockyLineGraph() {}
    void paint (juce::Graphics& g);
private:
    const int pixelSize;
};

#endif //MAIM_BLOCKYLINEGRAPH_H
