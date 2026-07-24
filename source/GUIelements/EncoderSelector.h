/*
  ==============================================================================

    EncoderSelector.h
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

//==============================================================================
/*
    A 3-row selector for the encoder choice parameter (Blade / Lame / Opus),
    styled to match the encoderButton ToggleButton look, drawn by
    MaimLookAndFeel::drawEncoderSelector.
*/
class EncoderSelector  : public juce::Component
{
public:
    EncoderSelector (juce::AudioProcessorValueTreeState& p, const juce::String& parameterID);
    ~EncoderSelector() override;

    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;

    int getSelectedIndex() const noexcept { return selectedIndex; }

    const juce::StringArray optionNames {"Blade", "Lame", "Opus"};

private:
    juce::RangedAudioParameter& parameter;
    std::unique_ptr<juce::ParameterAttachment> attachment;
    int selectedIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EncoderSelector)
};
