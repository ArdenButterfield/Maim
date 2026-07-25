/*
  ==============================================================================

    PresetsSection.cpp
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "PresetsSection.h"

//==============================================================================

PresetsSection::PresetsSection(MaimAudioProcessor& processor) :
        StageWindow(processor.getValueTreeState()),
        audioProcessor(processor),
        presetManager(processor.getPresetManager())
{
    sectionName.setColour(sectionName.textColourId, MaimColours::BEVEL_BLACK);
    sectionName.setFont(H1_font);
    sectionName.setText("Presets", juce::dontSendNotification);
    sectionName.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(sectionName);

    presetBox.setTextWhenNothingSelected("No preset loaded");
    presetBox.setTextWhenNoChoicesAvailable("No presets found");
    presetBox.onChange = [this] { presetBoxChanged(); };
    addAndMakeVisible(presetBox);

    previousPresetButton.onClick = [this] { goToAdjacentPreset(-1); };
    addAndMakeVisible(previousPresetButton);

    nextPresetButton.onClick = [this] { goToAdjacentPreset(1); };
    addAndMakeVisible(nextPresetButton);

    saveButton.onClick = [this] { promptAndSavePreset(); };
    addAndMakeVisible(saveButton);

    refreshPresetList();
}

void PresetsSection::refreshPresetList()
{
    presetBox.clear(juce::dontSendNotification);
    presetFiles.clear();

    auto addGroup = [this] (const juce::String& heading, const juce::Array<juce::File>& files)
    {
        if (files.isEmpty())
            return;

        presetBox.addSectionHeading(heading);
        for (const auto& file : files) {
            presetFiles.add(file);
            presetBox.addItem(file.getFileNameWithoutExtension(), presetFiles.size());
        }
    };

    addGroup("User", presetManager.getUserPresets());
    addGroup("Default", presetManager.getDefaultPresets());

    presetBox.setSelectedId(0, juce::dontSendNotification);
}

void PresetsSection::presetBoxChanged()
{
    const int id = presetBox.getSelectedId();
    if (id <= 0 || id > presetFiles.size())
        return;

    presetManager.loadPreset(presetFiles.getReference(id - 1));
}

void PresetsSection::goToAdjacentPreset(int direction)
{
    if (presetFiles.isEmpty())
        return;

    const int currentIndex = presetBox.getSelectedId() - 1;
    const int newIndex = juce::jlimit(0, presetFiles.size() - 1, currentIndex + direction);

    presetBox.setSelectedId(newIndex + 1);
}

void PresetsSection::promptAndSavePreset()
{
    auto* window = new juce::AlertWindow("Save Preset",
        "Enter a name for this preset:",
        juce::MessageBoxIconType::NoIcon);
    window->addTextEditor("name", "", "");
    window->addButton("Save", 1, juce::KeyPress(juce::KeyPress::returnKey));
    window->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));

    juce::Component::SafePointer<PresetsSection> safeThis(this);

    window->enterModalState(true,
        juce::ModalCallbackFunction::create(
            [safeThis, window](int result)
            {
                if (result == 1 && safeThis != nullptr) {
                    if (safeThis->presetManager.savePreset(window->getTextEditorContents("name")))
                        safeThis->refreshPresetList();
                }
            }),
        true);
}

void PresetsSection::paint (juce::Graphics& g)
{
    StageWindow::paint(g);
    g.setColour(BEVEL_LIGHT);
    g.fillRect(usable_bounds);
}

void PresetsSection::resized()
{
    setUsableBounds();

    auto bounds = usable_bounds.withTrimmedLeft(standardMargin).withTrimmedRight(standardMargin).reduced(0, 6);

    const int titleWidth = 90;
    const int buttonWidth = 28;
    const int saveButtonWidth = 70;
    const int gap = standardMargin;

    sectionName.setBounds(bounds.withWidth(titleWidth));

    auto controls = bounds.withTrimmedLeft(titleWidth + gap);

    saveButton.setBounds(controls.withWidth(saveButtonWidth).withRightX(controls.getRight()));

    auto navControls = controls.withTrimmedRight(saveButtonWidth + gap);

    nextPresetButton.setBounds(navControls.withWidth(buttonWidth).withRightX(navControls.getRight()));

    auto comboAndPrev = navControls.withTrimmedRight(buttonWidth + gap / 2);

    previousPresetButton.setBounds(comboAndPrev.withWidth(buttonWidth));

    presetBox.setBounds(comboAndPrev.withTrimmedLeft(buttonWidth + gap / 2));
}
