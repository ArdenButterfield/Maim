/*
  ==============================================================================

    PresetsSection.h
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_graphics/juce_graphics.h>

#include "StageWindow.h"
#include "../PluginProcessor.h"
#include "../PresetManager.h"

//==============================================================================
/*
*/
class PresetsSection  : public StageWindow
{
public:
    PresetsSection(MaimAudioProcessor& processor);
    void paint (juce::Graphics& g) override;
    void resized() override;
private:
    void refreshPresetList();
    void presetBoxChanged();
    void goToAdjacentPreset(int direction);
    void promptAndSavePreset();

    MaimAudioProcessor& audioProcessor;
    PresetManager& presetManager;

    juce::Label sectionName;
    juce::ComboBox presetBox;
    juce::TextButton previousPresetButton {"<"};
    juce::TextButton nextPresetButton {">"};
    juce::TextButton saveButton {"Save"};

    juce::Array<juce::File> presetFiles;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetsSection)
};
