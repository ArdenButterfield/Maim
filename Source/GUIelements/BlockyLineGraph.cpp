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
    std::vector<int> yPixels;
    int minYval, maxYval;
    int valIndex = 0;
    auto p = juce::Path();
    p.startNewSubPath(xVals[0], yVals[0]);
    g.setColour(lineColour.withAlpha(0.3f));


    for (int x = 0; x < getWidth(); x += pixelSize) {
        yPixels.resize(0);
        minYval = getHeight();
        maxYval = 0;
        for (; (valIndex < numPoints) && (xVals[valIndex] < x + pixelSize); ++valIndex) {
            // if (yVals[valIndex] == getHeight()) { continue }
            int yPixel = (int)yVals[valIndex];
            yPixel -= yPixel % pixelSize;
            if (yVals[valIndex] < getHeight()) {

                maxYval = yPixel > maxYval ? yPixel : maxYval;
            }
            minYval = yPixel < minYval ? yPixel : minYval;

//            if (std::find(yPixels.begin(), yPixels.end(), yPixel) == yPixels.end()) {
//                yPixels.push_back(yPixel);
//            }
        }
//        for (int y : yPixels) {
//            g.fillRect(x,y,pixelSize,pixelSize);
//        }
        if (maxYval >= minYval) {

            g.fillRect(x, minYval, pixelSize, maxYval - minYval + pixelSize);
        }

        p.lineTo(x,minYval);
        p.lineTo(x + pixelSize, minYval);
            /*for (int y = *std::min_element(yPixels.begin(), yPixels.end()); y <= *std::max_element(yPixels.begin(), yPixels.end()); ++y) {
                // g.fillRect(x, y, pixelSize, pixelSize);

            }*/
    }
    g.setColour(lineColour);

    g.strokePath(p, juce::PathStrokeType(1.f));

}