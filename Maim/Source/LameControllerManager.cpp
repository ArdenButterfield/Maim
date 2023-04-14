/*
  ==============================================================================

    LameControllerManager.cpp
    Created: 10 Apr 2023 5:05:43pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "LameControllerManager.h"

LameControllerManager::LameControllerManager(int s, int initialBitrate, int spb, juce::AudioProcessorValueTreeState& p) :
    samplerate(s),
    samplesPerBlock(spb),
    blocksBeforeSwitch(3000 / samplesPerBlock), // Lame encoding + decoding delay, conservative estimate based on https://lame.sourceforge.io/tech-FAQ.txt
    parameters(p)

{
    parametersNeedUpdating = false;
    
    parameters.addParameterListener("butterflystandard", this);
    parameters.addParameterListener("butterflycrossed", this);
    parameters.addParameterListener("mdctstep", this);
    parameters.addParameterListener("mdctinvert", this);
    parameters.addParameterListener("mdctposthshift", this);
    parameters.addParameterListener("mdctpostvshift", this);
    parameters.addParameterListener("mdctwindowincr", this);
    parameters.addParameterListener("mdctsampincr", this);
    parameters.addParameterListener("bitrate", this);
    parameters.addParameterListener("bitratesquish", this);
    parameters.addParameterListener("thresholdbias", this);
    parameters.addParameterListener("mdctfeedback", this);
    
    for (int i = 0; i < NUM_REASSIGNMENT_BANDS; ++i) {
        std::stringstream id;
        id << "bandorder" << i;
        parameters.addParameterListener(id.str(), this);
        bandReassignmentParameters[i] = (juce::AudioParameterInt*)parameters.getParameter(id.str());
    }

    
    controllers[0].init(samplerate, samplesPerBlock, initialBitrate);
    
    currentController = &controllers[0];
    offController = &controllers[1];
    
    currentBitrate = initialBitrate;
    wantingToSwitch = false;
    switchCountdown = 0;
    
    startTimerHz(30);
}

LameControllerManager::~LameControllerManager()
{
    parameters.removeParameterListener("butterflystandard", this);
    parameters.removeParameterListener("butterflycrossed", this);
    parameters.removeParameterListener("mdctstep", this);
    parameters.removeParameterListener("mdctinvert", this);
    parameters.removeParameterListener("mdctposthshift", this);
    parameters.removeParameterListener("mdctpostvshift", this);
    parameters.removeParameterListener("mdctwindowincr", this);
    parameters.removeParameterListener("mdctsampincr", this);
    parameters.removeParameterListener("bitrate", this);
    parameters.removeParameterListener("bitratesquish", this);
    parameters.removeParameterListener("thresholdbias", this);
    parameters.removeParameterListener("mdctfeedback", this);
    
    for (int i = 0; i < NUM_REASSIGNMENT_BANDS; ++i) {
        std::stringstream id;
        id << "bandorder" << i;
        parameters.removeParameterListener(id.str(), this);
    }
}

void LameControllerManager::parameterChanged (const juce::String &parameterID, float newValue)
{
    parametersNeedUpdating = true;
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
    if (parametersNeedUpdating) {
        updateParameters();
    }
    
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

void LameControllerManager::updateParameters(bool updateOffController)
{
    int bitrate = bitrates[((juce::AudioParameterChoice*) parameters.getParameter("bitrate"))->getIndex()];
    if (bitrate != getBitrate()) {
        changeBitrate(bitrate);
    }
    
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
    
    controller->setMDCTfeedback(
        ((juce::AudioParameterFloat*) parameters.getParameter("mdctfeedback"))->get()
    );
    
    controller->setMDCTpostshiftBends(
        ((juce::AudioParameterInt*) parameters.getParameter("mdctposthshift"))->get(),
       ((juce::AudioParameterFloat*) parameters.getParameter("mdctpostvshift"))->get()
    );
    controller->setMDCTwindowincrBends(
        ((juce::AudioParameterInt*) parameters.getParameter("mdctwindowincr"))->get()
    );
    controller->setBitrateSquishBends(
        ((juce::AudioParameterFloat*) parameters.getParameter("bitratesquish"))->get()
    );
    
     controller->setThresholdBias(((juce::AudioParameterFloat*) parameters.getParameter("thresholdbias"))->get()
    );
    
    int bandReassign[32];
    int i;
    for (i = 0; i < 20; ++i) {
        bandReassign[i] = bandReassignmentParameters[i]->get();
    }
    for (; i < 32; ++i) {
        bandReassign[i] = i;
    }
    controller->setMDCTBandReassignmentBends(bandReassign);

    
    if (wantingToSwitch && !updateOffController) {
        updateParameters(true);
    }
    auto psychoanalState = parameters.state.getChildWithName("psychoanal");
    auto indicator = psychoanalState.getProperty("shortblockindicator");
    bool shortBlockStatus = controller->getShortBlockStatus();
    if (!(indicator.isBool() && ((bool)indicator == shortBlockStatus))) {
        psychoanalState.setProperty("shortblockindicator", shortBlockStatus, nullptr);
    }
}

int LameControllerManager::getBitrate()
{
    return currentBitrate;
}

float* LameControllerManager::getPsychoanalEnergy()
{
    return currentController->getPsychoanalEnergy();
}

float* LameControllerManager::getPsychoanalThreshold()
{
    return currentController->getPsychoanalThreshold();
}

float rescalePsychoanal(const float a) {
    return log10(a > 1 ? a : 1) / 14;
}

void LameControllerManager::timerCallback()
{
    float* energy = getPsychoanalEnergy();
    float* threshold = getPsychoanalThreshold();
    
    juce::var thresholdV, energyV;
    for (int i = 0; i < 22; ++i) {
        thresholdV.append(rescalePsychoanal(threshold[i]));
        energyV.append(rescalePsychoanal(energy[i])); // TEMP test
    }

    auto psychoSpectrum = parameters.state.getChildWithName("psychoanal");
    psychoSpectrum.setProperty("threshold", thresholdV, nullptr);
    psychoSpectrum.setProperty("energy", energyV, nullptr);
}
