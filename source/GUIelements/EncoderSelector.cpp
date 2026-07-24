/*
  ==============================================================================

    EncoderSelector.cpp
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "EncoderSelector.h"
#include "MaimLookAndFeel.h"

EncoderSelector::EncoderSelector (juce::AudioProcessorValueTreeState& p, const juce::String& parameterID)
    : parameter (*p.getParameter (parameterID))
{
    attachment = std::make_unique<juce::ParameterAttachment> (parameter,
        [this] (float newValue)
        {
            selectedIndex = juce::roundToInt (newValue);
            repaint();
        });
    attachment->sendInitialUpdate();
}

EncoderSelector::~EncoderSelector() = default;

void EncoderSelector::paint (juce::Graphics& g)
{
    if (auto* lnf = dynamic_cast<MaimLookAndFeel*> (&getLookAndFeel()))
        lnf->drawEncoderSelector (g, *this);
}

void EncoderSelector::mouseUp (const juce::MouseEvent& e)
{
    const auto numOptions = optionNames.size();
    if (numOptions == 0 || getHeight() <= 0)
        return;

    const auto row = juce::jlimit (0, numOptions - 1,
        static_cast<int> ((static_cast<float> (e.y) / static_cast<float> (getHeight())) * (float) numOptions));

    if (row != selectedIndex)
        attachment->setValueAsCompleteGesture ((float) row);
}
