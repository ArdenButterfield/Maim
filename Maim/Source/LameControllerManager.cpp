/*
  ==============================================================================

    LameControllerManager.cpp
    Created: 10 Apr 2023 5:05:43pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "LameControllerManager.h"

LameControllerManager::LameControllerManager(int s, int initialBitrate, int spb) :
    samplerate(s),
    samplesPerBlock(spb),
    blocksBeforeSwitch(3000 / samplesPerBlock) // Lame encoding + decoding delay, conservative estimate based on https://lame.sourceforge.io/tech-FAQ.txt

{
    controllers[0].init(samplerate, samplesPerBlock, initialBitrate);
    
    currentController = &controllers[0];
    offController = &controllers[1];
    
    currentBitrate = initialBitrate;
    wantingToSwitch = false;
    switchCountdown = 0;
}

LameControllerManager::~LameControllerManager()
{

}

void LameControllerManager::changeBitrate(int new_bitrate)
{
    if (wantingToSwitch && (new_bitrate == offController->getBitrate())) {
        return;
    }
    if (new_bitrate == currentBitrate) {
        wantingToSwitch = false;
        return;
        
    }
    offController->init(samplerate, samplesPerBlock, new_bitrate);
    wantingToSwitch = true;
    switchCountdown = blocksBeforeSwitch;
}

void LameControllerManager::processBlock(juce::AudioBuffer<float>& buffer)
{
    
    if (buffer.getNumChannels() != 2) {
        return;
    }
    auto samplesL = buffer.getWritePointer(0);
    auto samplesR = buffer.getWritePointer(1);
    
    currentController->addNextInput(samplesL, samplesR, buffer.getNumSamples());
    if (switchCountdown > 0) {

        offController->addNextInput(samplesL, samplesR, buffer.getNumSamples());
        if (offController->copyOutput(nullptr, nullptr, buffer.getNumSamples())) {
            --switchCountdown;
        }
    } else if (wantingToSwitch) {
        offController->addNextInput(samplesL, samplesR, buffer.getNumSamples());
        if (offController->copyOutput(samplesL, samplesR, buffer.getNumSamples())) {
            auto tempBuffer = juce::AudioBuffer<float>(buffer.getNumChannels(),
                                                       buffer.getNumSamples());
            samplesL = tempBuffer.getWritePointer(0);
            samplesR = tempBuffer.getWritePointer(1);
            currentController->copyOutput(samplesL, samplesR, buffer.getNumSamples());
            
            buffer.applyGainRamp(0, buffer.getNumSamples(), 0, 1);
            buffer.addFromWithRamp(0, 0, samplesL, buffer.getNumSamples(), 1, 0);
            buffer.addFromWithRamp(1, 0, samplesR, buffer.getNumSamples(), 1, 0);
            
            auto temp = currentController;
            currentController = offController;
            offController = temp;
            wantingToSwitch = false;
            return;
        }
    }
    
    if (!currentController->copyOutput(samplesL, samplesR, buffer.getNumSamples())) {
        memset(samplesL, 0, sizeof(float) * buffer.getNumSamples());
        memset(samplesR, 0, sizeof(float) * buffer.getNumSamples());
    }
}

void LameControllerManager::updateParameters(juce::AudioProcessorValueTreeState& parameters,
                                             std::array<juce::AudioParameterInt*, 20>* bandReassignmentParameters,
                                             bool updateOffController)
{
    auto controller = updateOffController ? offController : currentController;
    
    controller->setButterflyBends(
        ((juce::AudioParameterFloat*) parameters.getParameter("butterflystandard"))->get(),
        ((juce::AudioParameterFloat*) parameters.getParameter("butterflycrossed"))->get(),
        ((juce::AudioParameterFloat*) parameters.getParameter("butterflycrossed"))->get(),
        ((juce::AudioParameterFloat*) parameters.getParameter("butterflystandard"))->get()
    );
    
    controller->setMDCTbandstepBends(
        ((juce::AudioParameterBool*) parameters.getParameter("mdctinvert"))->get(),
        ((juce::AudioParameterInt*) parameters.getParameter("mdctstep"))->get()
    );
    
    controller->setMDCTpostshiftBends(
         ((juce::AudioParameterInt*) parameters.getParameter("mdctposthshift"))->get(),
         ((juce::AudioParameterFloat*) parameters.getParameter("mdctpostvshift"))->get()
    );
    
    controller->setMDCTwindowincrBends(
         ((juce::AudioParameterInt*) parameters.getParameter("mdctwindowincr"))->get(),
         ((juce::AudioParameterInt*) parameters.getParameter("mdctsampincr"))->get()
    );
    
    int bandReassign[32];
    int i;
    for (i = 0; i < 20; ++i) {
        bandReassign[i] = (*bandReassignmentParameters)[i]->get();
    }
    for (; i < 32; ++i) {
        bandReassign[i] = i;
    }
    controller->setMDCTBandReassignmentBends(bandReassign);

    
    if (wantingToSwitch && !updateOffController) {
        updateParameters(parameters, bandReassignmentParameters, true);
    }
}

int LameControllerManager::getBitrate()
{
    return currentBitrate;
}
