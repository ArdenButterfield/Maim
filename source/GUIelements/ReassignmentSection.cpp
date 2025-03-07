/*
  ==============================================================================

    ReassignmentSection.cpp
    Created: 9 Apr 2023 7:03:21pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>
#include "../parameterIds.h"
#include <cmath>

#include "ReassignmentSection.h"

//==============================================================================
ReassignmentSection::ReassignmentSection (juce::AudioProcessorValueTreeState& p, int numItems, int s) :
    StageWindow(p),
    resetButton("reset", RESET),
    randomButton("random", RANDOM),
    upButton("up", SHIFT_UP),
    downButton("down", SHIFT_DOWN),
    pTree(p)
{
    sectionName.setColour(juce::Label::textColourId, MaimColours::BEVEL_BLACK);
    sectionName.setFont(sectionNameFont);
    sectionName.setText("Frequency Reassignment", juce::dontSendNotification);
    sectionName.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(mp3OnlyLabel);
    addAndMakeVisible(resetButton);
    addAndMakeVisible(randomButton);
    addAndMakeVisible(upButton);
    addAndMakeVisible(downButton);
    addAndMakeVisible(sectionName);

    pTree.addParameterListener(ENCODER_PARAM_ID, this);
    bool isOpus = (((juce::AudioParameterChoice*)pTree.getParameter(ENCODER_PARAM_ID))->getIndex() == 2);
    resetButton.setVisible(!isOpus);
    randomButton.setVisible(!isOpus);
    upButton.setVisible(!isOpus);
    downButton.setVisible(!isOpus);
    mp3OnlyLabel.setVisible(isOpus);

    resetButton.addListener(this);
    randomButton.addListener(this);
    upButton.addListener(this);
    downButton.addListener(this);
    
    parameters.resize(numItems);
    for (unsigned i = 0; i < numItems; ++i) {
        pTree.addParameterListener(BAND_ORDER_PARAM_IDS[i], this);
        parameters[i] = (juce::AudioParameterInt*)pTree.getParameter(BAND_ORDER_PARAM_IDS[i]);
    }
    itemVals.resize(numItems);
    
    for (unsigned i = 0; i < numItems; ++i) {
        itemVals[i] = parameters[i]->get();
    }
    steps = s;

    startTimerHz(30);
    needsRepainting = false;
    
}

ReassignmentSection::~ReassignmentSection()
{
    for (const auto & i : BAND_ORDER_PARAM_IDS) {
        pTree.removeParameterListener(i, this);
    }
    
    resetButton.removeListener(this);
    randomButton.removeListener(this);
    upButton.removeListener(this);
    downButton.removeListener(this);

    pTree.removeParameterListener(ENCODER_PARAM_ID, this);
}

void ReassignmentSection::setValue(const int index, const int newVal)
{
    if ((index < 0) || (index >= itemVals.size())) {
        return;
    }
    itemVals[index] = juce::jmax(0, juce::jmin(newVal, steps - 1));
    (*parameters[index]) = newVal;
}

void ReassignmentSection::resetGraph()
{
    for (int i = 0; i < itemVals.size(); ++i) {
        setValue(i, i);
    }
    repaint();
}

void ReassignmentSection::randomizeGraph()
{
    for (int i = 0; i < itemVals.size(); ++i) {
        setValue(i, random.nextInt(itemVals.size()));
    }
    repaint();
}

void ReassignmentSection::shiftGraph(bool up)
{
    int inc = up ? 1 : -1;
    for (int i = 0; i < itemVals.size(); ++i) {
        setValue(i, itemVals[i] + inc);
    }
    repaint();
}

void ReassignmentSection::updateChart(const juce::Point<float>& mousePosition, bool strictBounds)
{
    float x, y;
    x = mousePosition.getX();
    y = mousePosition.getY();
    if (!activeArea.contains((int)x,(int)y)) {
        if (strictBounds) {
            return;
        } else {
            x = juce::jmin(juce::jmax(x, (float)activeArea.getX()),
                           (float)activeArea.getRight());
            y = juce::jmin(juce::jmax(y, (float)activeArea.getY()),
                           (float)activeArea.getBottom());
        }
    }
    int i = (x - activeArea.getX()) * itemVals.size() / activeArea.getWidth();
    setValue(i, getValIndex((y)));
    
    repaint();
}

void ReassignmentSection::mouseDown(const juce::MouseEvent& event)
{
    updateChart(event.position, true);
}

void ReassignmentSection::mouseDrag(const juce::MouseEvent& event)
{
    updateChart(event.position, false);
}

void ReassignmentSection::mouseWheelMove(const juce::MouseEvent &event, const juce::MouseWheelDetails &wheel)
{
    if (!activeArea.contains((int)event.position.getX(),(int)event.position.getY())) {
        return;
    }

    int inc;
    if (wheel.deltaY > 0) {
        inc = 1;
    } else if (wheel.deltaY < 0) {
        inc = -1;
    } else {
        return;
    }


    int i = (event.position.getX() - activeArea.getX()) * itemVals.size() / activeArea.getWidth();

    setValue(i, itemVals[i] + inc);
}


float ReassignmentSection::getValScreenY(const int rawVal)
{
    return activeArea.getHeight() * (1 - ((float)rawVal / (steps))) + activeArea.getY();
}

int ReassignmentSection::getValIndex(const float screenY)
{
    float v = 1 - ((screenY - activeArea.getY()) / activeArea.getHeight());
    return (int) std::round(v * (steps));
}

void ReassignmentSection::paint (juce::Graphics& g)
{
    StageWindow::paint(g);

    if (mp3OnlyLabel.isVisible()) {
        return;
    }
    g.setColour(MaimColours::BEVEL_LIGHT);
    g.fillRect(activeAreaBorder);
    g.setColour(MaimColours::BEVEL_BLACK);
    g.drawRect(activeAreaBorder, 2);
    float singleLineWidth = (float)activeArea.getWidth() / itemVals.size();
    g.setColour(MaimColours::PANEL_BACKGROUND_COLOR);
    for (int row = 0; row < steps + 1; ++row) {
        g.drawHorizontalLine(getValScreenY(row), activeArea.getX(), activeArea.getRight());
    }
    for (int col = 0; col < itemVals.size() + 1; ++col) {
        g.drawVerticalLine(activeArea.getX() + singleLineWidth * col, activeArea.getY(), activeArea.getBottom());
    }
    g.setColour(MaimColours::PANEL_BACKGROUND_COLOR);
    for (int i = 0; i < itemVals.size(); ++i) {
        int left = activeArea.getX() + singleLineWidth * i;
        int right = activeArea.getX() + singleLineWidth * (i + 1);
        int top = getValScreenY(i + 1);
        int bottom = getValScreenY(i);
        g.fillRect(left, top, right - left, bottom - top);
    }

    g.setColour(MaimColours::SPLASH_COLOR_DARK.withAlpha(0.7f));
    for (int i = 0; i < itemVals.size(); ++i) {
        int left = activeArea.getX() + singleLineWidth * i;
        int right = activeArea.getX() + singleLineWidth * (i + 1);
        int top = getValScreenY(itemVals[i] + 1);
        int bottom = getValScreenY(itemVals[i]);
        g.fillRect(left, top, right - left, bottom - top);
    }
    needsRepainting = false;
}

void ReassignmentSection::resized()
{
    // Buttons overlap with each other, and with the top edge of the active area.
    int buttonWidth = (getWidth() - standardMargin * 2 + 6) / 4;
    int mainRectWidth = buttonWidth * 4 - 6;
    auto mainRect = getLocalBounds().withSizeKeepingCentre(mainRectWidth, getHeight()).withTrimmedBottom(standardMargin);
    const auto buttonHeight = 40;
    const auto titleHeight = headerHeight;
    sectionName.setBounds(mainRect.withHeight(titleHeight));
    mainRect = mainRect.withTrimmedTop(titleHeight);
    int buttonDeltaX = buttonWidth - 2;
    auto buttonRect = mainRect.withHeight(buttonHeight);

    resetButton.setBounds(buttonRect.withWidth(buttonWidth));
    randomButton.setBounds(buttonRect.withWidth(buttonWidth).translated(buttonDeltaX, 0));
    upButton.setBounds(buttonRect.withWidth(buttonWidth).translated(buttonDeltaX * 2, 0));
    downButton.setBounds(buttonRect.withWidth(buttonWidth).translated(buttonDeltaX * 3, 0));

    activeAreaBorder = mainRect.withTrimmedTop(buttonHeight - 2);
    activeArea = activeAreaBorder
                     .withTrimmedTop(10)
                     .withTrimmedLeft(10)
                     .withTrimmedRight(10)
                     .withTrimmedBottom(10);

    mp3OnlyLabel.setBounds(getLocalBounds().withTrimmedTop(20));

}

void ReassignmentSection::parameterChanged (const juce::String &parameterID, float newValue)
{
    buildItemValsFromParams();

    bool isOpus = (((juce::AudioParameterChoice*)pTree.getParameter(ENCODER_PARAM_ID))->getIndex() == 2);
    resetButton.setVisible(!isOpus);
    randomButton.setVisible(!isOpus);
    upButton.setVisible(!isOpus);
    downButton.setVisible(!isOpus);
    mp3OnlyLabel.setVisible(isOpus);


    needsRepainting = true;
}

void ReassignmentSection::buildItemValsFromParams()
{
    for (int i = 0; i < itemVals.size(); ++i) {
        setValue(i, parameters[i]->get());
    }
}

void ReassignmentSection::timerCallback()
{
    if (needsRepainting) {
        repaint();
    }
}

void ReassignmentSection::buttonClicked (juce::Button * b)
{
    if (b == &resetButton) {
        resetGraph();
    } else if (b == &randomButton) {
        randomizeGraph();
    } else if (b == &upButton) {
        shiftGraph(true);
    } else if (b == &downButton) {
        shiftGraph(false);
    }
}
void ReassignmentSection::buttonStateChanged (juce::Button * b)
{
}
