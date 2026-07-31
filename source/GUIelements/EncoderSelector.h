/*
  ==============================================================================

    EncoderSelector.h
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>
#include "MaimColours.h"
#include <vector>

//==============================================================================
/*
    A vertical, multi-row selector for a choice parameter (e.g. the
    Blade / Lame / Opus encoder choice), styled to match the encoderButton
    ToggleButton look, drawn by MaimLookAndFeel::drawEncoderSelector.
*/
class EncoderSelector  : public juce::Component
{
public:
    EncoderSelector (juce::AudioProcessorValueTreeState& p, const juce::String& parameterID,
        juce::StringArray optionNames = {"Blade", "Lame", "Opus"},
        std::vector<juce::Colour> optionColours = {
            MaimColours::CONTRAST_COLOR_LIGHT.withLightness (0.65),
            MaimColours::SPLASH_COLOR_LIGHT,
            MaimColours::OPUS_COLOR_LIGHT});
    ~EncoderSelector() override;

    void paint (juce::Graphics&) override;
    void mouseUp (const juce::MouseEvent&) override;

    int getSelectedIndex() const noexcept { return selectedIndex; }

    const juce::StringArray optionNames;
    const std::vector<juce::Colour> optionColours;

private:
    juce::RangedAudioParameter& parameter;
    std::unique_ptr<juce::ParameterAttachment> attachment;
    int selectedIndex = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EncoderSelector)
};
