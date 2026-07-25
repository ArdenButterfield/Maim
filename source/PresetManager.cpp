/*
  ==============================================================================

    PresetManager.cpp
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "PresetManager.h"
#include "PluginProcessor.h"

namespace {
    struct FileNameComparator
    {
        static int compareElements(const juce::File& a, const juce::File& b)
        {
            return a.getFileNameWithoutExtension().compareIgnoreCase(b.getFileNameWithoutExtension());
        }
    };
}

const juce::String PresetManager::presetFileExtension = ".maimpreset";

PresetManager::PresetManager(MaimAudioProcessor& processorToUse) : processor(processorToUse)
{
}

juce::File PresetManager::getMaimDataDirectory()
{
    return juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory)
        .getChildFile("MaimData");
}

juce::File PresetManager::getUserPresetsDirectory()
{
    return getMaimDataDirectory().getChildFile("User");
}

juce::File PresetManager::getDefaultPresetsDirectory()
{
    return getMaimDataDirectory().getChildFile("Default");
}

void PresetManager::createPresetDirectories()
{
    getUserPresetsDirectory().createDirectory();
    getDefaultPresetsDirectory().createDirectory();
}

juce::Array<juce::File> PresetManager::getPresetsIn(const juce::File& directory)
{
    juce::Array<juce::File> presets;
    if (directory.isDirectory()) {
        for (const auto& file : directory.findChildFiles(
                 juce::File::findFiles, false, "*" + presetFileExtension)) {
            presets.add(file);
        }
    }

    FileNameComparator comparator;
    presets.sort(comparator);

    return presets;
}

juce::Array<juce::File> PresetManager::getUserPresets() const
{
    return getPresetsIn(getUserPresetsDirectory());
}

juce::Array<juce::File> PresetManager::getDefaultPresets() const
{
    return getPresetsIn(getDefaultPresetsDirectory());
}

bool PresetManager::savePreset(const juce::String& presetName) const
{
    if (presetName.isEmpty())
        return false;

    auto file = getUserPresetsDirectory().getChildFile(
        juce::File::createLegalFileName(presetName) + presetFileExtension);

    juce::MemoryBlock state;
    processor.getStateInformation(state);

    return file.replaceWithData(state.getData(), state.getSize());
}

bool PresetManager::loadPreset(const juce::File& presetFile) const
{
    juce::MemoryBlock state;
    if (!presetFile.existsAsFile() || !presetFile.loadFileAsData(state))
        return false;

    processor.setStateInformation(state.getData(), (int) state.getSize());
    return true;
}
