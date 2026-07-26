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

void EncoderSelector::mouseUp (const juce::MouseEvent&)
{
    const auto numOptions = optionNames.size();
    if (numOptions == 0)
        return;

    const auto nextIndex = (selectedIndex + 1) % numOptions;
    attachment->setValueAsCompleteGesture ((float) nextIndex);
}
