//
// Created by arden on 1/29/25.
//

#include "OpusPacketLossSection.h"
#include "../parameterIds.h"

OpusPacketLossSection::OpusPacketLossSection (juce::AudioProcessorValueTreeState& p)
    : StageWindow (p), packetLossDragBox(p, PACKET_LOSS_RATE_PARAM_ID, PACKET_LOSS_PULSE_WIDTH_PARAM_ID),
      jitterSlider(p, PACKET_LOSS_JITTER_PARAM_ID, "Jitter"), lossStickToggle("LOSS|STICK"),
      lossStickAttachment(p, PACKET_LOSS_STICK_PARAM_ID, lossStickToggle),
      packetLossModeSelector(p, PACKET_LOSS_MODE_PARAM_ID, {"Off", "Pulse", "On"},
          {MaimColours::OPUS_COLOR_LIGHT, MaimColours::OPUS_COLOR_LIGHT, MaimColours::OPUS_COLOR_LIGHT})
{
    jitterSlider.slider.getProperties().set("drawGrayArc", true);
    jitterSlider.slider.setRepaintsOnMouseActivity(true);

    lossStickToggle.setButtonText("LOSS|STICK");
    sectionName.setColour(sectionName.textColourId, MaimColours::BEVEL_BLACK);
    sectionName.setFont(sectionNameFont);
    sectionName.setText("Packet loss", juce::dontSendNotification);
    sectionName.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(sectionName);
    addAndMakeVisible(packetLossDragBox);
    addAndMakeVisible(jitterSlider);
    addAndMakeVisible(lossStickToggle);
    addAndMakeVisible(packetLossModeSelector);

    parameters.addParameterListener(PACKET_LOSS_STICK_PARAM_ID, this);
    parameters.addParameterListener(PACKET_LOSS_MODE_PARAM_ID, this);

    packetLossDragBox.setColourScheme(!*(juce::AudioParameterBool*)parameters.getParameter(PACKET_LOSS_STICK_PARAM_ID));
    packetLossDragBox.setPacketLossMode(((juce::AudioParameterChoice*)parameters.getParameter(PACKET_LOSS_MODE_PARAM_ID))->getIndex());
}

OpusPacketLossSection::~OpusPacketLossSection() {
    parameters.removeParameterListener(PACKET_LOSS_STICK_PARAM_ID, this);
    parameters.removeParameterListener(PACKET_LOSS_MODE_PARAM_ID, this);
}

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

    auto toggleColumn = jitterSlider.getBounds().withRightX(mainPart.getRight()).withTrimmedTop(10);
    const int toggleHeight = 40;
    const int toggleGap = 6;
    lossStickToggle.setBounds(toggleColumn.withHeight(toggleHeight));
    packetLossModeSelector.setBounds(toggleColumn.withY(lossStickToggle.getBottom() + toggleGap).withHeight(toggleHeight));
}

void OpusPacketLossSection::paint (juce::Graphics& g)
{
    StageWindow::paint(g);
    
}

void OpusPacketLossSection::parameterChanged (const juce::String&, float)
{
    packetLossDragBox.setColourScheme(!*(juce::AudioParameterBool*)parameters.getParameter(PACKET_LOSS_STICK_PARAM_ID));
    packetLossDragBox.setPacketLossMode(((juce::AudioParameterChoice*)parameters.getParameter(PACKET_LOSS_MODE_PARAM_ID))->getIndex());
}
