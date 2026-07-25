/*
  ==============================================================================

    PresetManager.h
    Author:  Arden Butterfield

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class MaimAudioProcessor;

//==============================================================================
/*
    Reads and writes presets (records of plugin state, as produced by
    MaimAudioProcessor::getStateInformation) to disk.

    Presets live in a per-user "MaimData" folder, inside two subfolders:
    "User", for presets saved by the user, and "Default", for presets
    shipped with the plugin (not yet implemented).
*/
class PresetManager
{
public:
    explicit PresetManager(MaimAudioProcessor& processorToUse);

    static juce::File getMaimDataDirectory();
    static juce::File getUserPresetsDirectory();
    static juce::File getDefaultPresetsDirectory();

    // Called from prepareToPlay, so the folders are guaranteed to exist
    // by the time anything tries to read from or write to them.
    static void createPresetDirectories();

    static const juce::String presetFileExtension;

    juce::Array<juce::File> getUserPresets() const;
    juce::Array<juce::File> getDefaultPresets() const;

    bool savePreset(const juce::String& presetName) const;
    bool loadPreset(const juce::File& presetFile) const;

private:
    static juce::Array<juce::File> getPresetsIn(const juce::File& directory);

    MaimAudioProcessor& processor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
