//
// Created by arden on 1/29/25.
//

#include "OpusPacketLossSection.h"
#include "../parameterIds.h"

OpusPacketLossSection::OpusPacketLossSection (juce::AudioProcessorValueTreeState& p)
    : StageWindow (p), packetLossDragBox(p, PACKET_LOSS_RATE_PARAM_ID, PACKET_LOSS_PULSE_WIDTH_PARAM_ID),
      jitterSlider(p, PACKET_LOSS_JITTER_PARAM_ID, "Jitter"), lossStickToggle("LOSS|STICK"),
      lossStickAttachment(p, PACKET_LOSS_STICK_PARAM_ID, lossStickToggle)
{
    lossStickToggle.setButtonText("LOSS|STICK");
    sectionName.setColour(sectionName.textColourId, MaimColours::BEVEL_BLACK);
    sectionName.setFont(sectionNameFont);
    sectionName.setText("Packet loss", juce::dontSendNotification);
    sectionName.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(sectionName);
    addAndMakeVisible(packetLossDragBox);
    addAndMakeVisible(jitterSlider);
    addAndMakeVisible(lossStickToggle);
}

OpusPacketLossSection::~OpusPacketLossSection()
= default;

void OpusPacketLossSection::resized()
{
    const int titleHeight = headerHeight;

    auto mainPart = getLocalBounds()
                        .withTrimmedLeft(standardMargin)
                        .withTrimmedRight(standardMargin)
                        .withTrimmedBottom(standardMargin);

    sectionName.setBounds(mainPart.withHeight(titleHeight));
    mainPart = mainPart.withTrimmedTop(titleHeight);

    auto dragboxPart = mainPart.withHeight(mainPart.getHeight() / 2);
    packetLossDragBox.setBounds(dragboxPart);
    jitterSlider.setBounds(mainPart.withTop(dragboxPart.getBottom()).withWidth(mainPart.getWidth() / 2));
    lossStickToggle.setBounds(jitterSlider.getBounds().withRightX(mainPart.getRight()).withHeight(40));
}

void OpusPacketLossSection::paint (juce::Graphics& g)
{
    StageWindow::paint(g);
    
}

void OpusPacketLossSection::parameterChanged (const juce::String& parameterID, float newValue)
{
}
