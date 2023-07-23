//
// Created by arden on 7/22/23.
//

#include "BlockyLineGraph.h"

#include <algorithm>

BlockyLineGraph::BlockyLineGraph(float y_min, float y_max, int num_points, juce::Colour c, int _pixelSize)
    : LineGraph<float>(y_min, y_max, num_points, c), pixelSize(_pixelSize)
{}

void BlockyLineGraph::paint (juce::Graphics& g)
{
    g.setColour(lineColour);
    std::vector<int> yPixels;
    int valIndex = 0;
    for (int x = 0; x < getWidth(); x += pixelSize) {
        yPixels.resize(0);
        for (; (valIndex < numPoints) && (xVals[valIndex] < x + pixelSize); ++valIndex) {
            // if (yVals[valIndex] == getHeight()) { continue }
            int yPixel = (int)yVals[valIndex];
            yPixel -= yPixel % pixelSize;
            yPixels.push_back(yPixel);

//            if (std::find(yPixels.begin(), yPixels.end(), yPixel) == yPixels.end()) {
//                yPixels.push_back(yPixel);
//            }
        }
        if (yPixels.size() > 0) {
            for (int y = *std::min_element(yPixels.begin(), yPixels.end()); y <= *std::max_element(yPixels.begin(), yPixels.end()); ++y) {
                g.fillRect(x, y, pixelSize, pixelSize);
            }

        }
    }
}