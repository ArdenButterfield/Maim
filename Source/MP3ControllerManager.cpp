/*
  ==============================================================================

    MP3ControllerManager.cpp
    Created: 10 Apr 2023 5:05:43pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "MP3ControllerManager.h"

void fadeTowards(float* currentBuffer, float* newBuffer, int numSamples) {
    // NOTE: should potentially be an equal-power crossfade, to prevent a dip in volume.
    // Juce applyGainRamp uses a linear ramp as well, though.
    for (int i = 0; i < numSamples; ++i) {
        float inc = (float)i / (float)numSamples;
        currentBuffer[i] = inc * newBuffer[i] + (1 - inc) * currentBuffer[i];
    }
}


MP3ControllerManager::MP3ControllerManager(juce::AudioProcessorValueTreeState& p) :
    parameters(p),
    currentEncoder(lame),
    currentControllerIndex(0)
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
    parameters.addParameterListener("encoder", this);
    
    for (int i = 0; i < NUM_REASSIGNMENT_BANDS; ++i) {
        std::stringstream id;
        id << "bandorder" << i;
        parameters.addParameterListener(id.str(), this);
        bandReassignmentParameters[i] = (juce::AudioParameterInt*)parameters.getParameter(id.str());
    }
}

MP3ControllerManager::~MP3ControllerManager()
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
    parameters.removeParameterListener("encoder", this);

    for (int i = 0; i < NUM_REASSIGNMENT_BANDS; ++i) {
        std::stringstream id;
        id << "bandorder" << i;
        parameters.removeParameterListener(id.str(), this);
    }
}

void MP3ControllerManager::initialize (int _samplerate, int _initialBitrate, int _samplesPerBlock)
{
    samplerate = _samplerate;
    samplesPerBlock = _samplesPerBlock;

    // useful for debugging
    lameControllers[0].name = "lame0";
    lameControllers[1].name = "lame1";
    bladeControllers[0].name = "blade0";
    bladeControllers[1].name = "blade1";

    if (currentEncoder == lame) {
        currentController = &(lameControllers[currentControllerIndex]);
    } else {
        currentController = &(bladeControllers[currentControllerIndex]);
    }
    currentController->init(samplerate, samplesPerBlock, _initialBitrate);
    offController = nullptr;

    currentBitrate = _initialBitrate;
    wantingToSwitch = false;
    switchCountdown = 0;

    inputBufferL = std::make_unique<QueueBuffer<float>>(MP3FRAMESIZE + samplesPerBlock, 0.f);
    inputBufferR = std::make_unique<QueueBuffer<float>>(MP3FRAMESIZE + samplesPerBlock, 0.f);
    outputBufferL = std::make_unique<QueueBuffer<float>>(MP3FRAMESIZE + samplesPerBlock, 0.f);
    outputBufferR = std::make_unique<QueueBuffer<float>>(MP3FRAMESIZE + samplesPerBlock, 0.f);

    for (auto i = 0; i < MP3FRAMESIZE; ++i) {
        inputBufferL->enqueue(0);
        inputBufferR->enqueue(0);
    }

    startTimerHz(30);
}

void MP3ControllerManager::parameterChanged (const juce::String &parameterID, float newValue)
{
    parametersNeedUpdating = true;
}

void MP3ControllerManager::changeController(int bitrate, Encoder encoder)
{
    if ((bitrate == currentBitrate) && (encoder == currentEncoder)) {
        wantingToSwitch = false;
        offController = nullptr;
        return;
    }
    if (wantingToSwitch && (bitrate == desiredBitrate) && (encoder == desiredEncoder)) {
        return;
    }
    desiredBitrate = bitrate;
    
    int offIndex = (currentControllerIndex + 1) % 2;
    
    if (encoder == lame) {
        desiredEncoder = lame;
        offController = &(lameControllers[offIndex]);
    } else {
        desiredEncoder = blade;
        offController = &(bladeControllers[offIndex]);
    }
    
    offController->init(samplerate, samplesPerBlock, desiredBitrate);
    wantingToSwitch = true;

}

void MP3ControllerManager::processBlock(juce::AudioBuffer<float>& buffer)
{
    if (parametersNeedUpdating) {
        updateParameters();
    }
    
    if (buffer.getNumChannels() != 2) {
        return;
    }
    auto samplesL = buffer.getWritePointer(0);
    auto samplesR = buffer.getWritePointer(1);

    for (auto s = 0; s < buffer.getNumSamples(); ++s) {
        inputBufferL->enqueue(samplesL[s]);
    }
    for (auto s = 0; s < buffer.getNumSamples(); ++s) {
        inputBufferR->enqueue(samplesR[s]);
    }
    float frameIn[2][MP3FRAMESIZE];
    float frameOut[2][MP3FRAMESIZE];
    while (inputBufferL->num_items() >= MP3FRAMESIZE) {
        for (auto s = 0; s < MP3FRAMESIZE; ++s) {
            frameIn[0][s] = inputBufferL->dequeue();
        }
        for (auto s = 0; s < MP3FRAMESIZE; ++s) {
            frameIn[1][s] = inputBufferR->dequeue();
        }
        if (wantingToSwitch) {
            float frameOutNew[2][MP3FRAMESIZE];
            offController->processFrame(previousFrame[0], previousFrame[1], nullptr, nullptr);
            offController->processFrame(frameIn[0], frameIn[1], frameOutNew[0], frameOutNew[1]);
            currentController->processFrame(frameIn[0], frameIn[1], frameOut[0], frameOut[1]);
            fadeTowards (frameOut[0], frameOutNew[0], MP3FRAMESIZE);
            fadeTowards(frameOut[1], frameOutNew[1], MP3FRAMESIZE);
            currentController = offController;
            currentBitrate = desiredBitrate;
            currentEncoder = desiredEncoder;
            offController = nullptr;
            currentControllerIndex = (currentControllerIndex + 1) % 2;
            wantingToSwitch = false;
        } else {
            currentController->processFrame(frameIn[0], frameIn[1], frameOut[0], frameOut[1]);
        }
        for (auto s = 0; s < MP3FRAMESIZE; ++s) {
            outputBufferL->enqueue(frameOut[0][s]);
        }
        for (auto s = 0; s < MP3FRAMESIZE; ++s) {
            outputBufferR->enqueue(frameOut[1][s]);
        }
        std::memcpy(previousFrame, frameIn, 2 * MP3FRAMESIZE * sizeof(float));
    }

    for (auto s = 0; s < buffer.getNumSamples(); ++s) {
        samplesL[s] = outputBufferL->dequeue();
    }
    for (auto s = 0; s < buffer.getNumSamples(); ++s) {
        samplesR[s] = outputBufferR->dequeue();
    }
}

void MP3ControllerManager::updateParameters()
{
    auto encoder = (Encoder)((juce::AudioParameterChoice*)
                                parameters.getParameter("encoder"))->getIndex();
    int bitrate = bitrates[((juce::AudioParameterChoice*)
                            parameters.getParameter("bitrate"))->getIndex()];
    changeController(bitrate, encoder);

    for (auto controller : {offController, currentController}) {
        if (controller == nullptr) {
            continue;
        }
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
    }

    auto psychoanalState = parameters.state.getChildWithName("psychoanal");
    auto indicator = psychoanalState.getProperty("shortblockindicator");
    bool shortBlockStatus = currentController->getShortBlockStatus();
    if (!(indicator.isBool() && ((bool)indicator == shortBlockStatus))) {
        psychoanalState.setProperty("shortblockindicator", shortBlockStatus, nullptr);
    }


    
    parametersNeedUpdating = false;
}

int MP3ControllerManager::getBitrate()
{
    return currentBitrate;
}

float* MP3ControllerManager::getPsychoanalEnergy()
{
    return currentController->getPsychoanalEnergy();
}

float* MP3ControllerManager::getPsychoanalThreshold()
{
    return currentController->getPsychoanalThreshold();
}

float* MP3ControllerManager::getMDCTpreBend()
{
    return currentController->getMDCTpreBend();
}

float* MP3ControllerManager::getMDCTpostBend()
{
    return currentController->getMDCTpostBend();
}

float rescalePsychoanal(const float a) {
    return log10(a > 1 ? a : 1) / 14;
}

float rescaleMDCT(const float a) {
    // mdct is 0 to 1(+), rescale to 0 to 1 but log scale
    if (a < (pow(10.f, -10.f))) {
        return 0;
    } else if (a > 1) {
        return 1;
    } else {
        return log10(a) / 10 + 1;
    }
}

void MP3ControllerManager::timerCallback()
{
    float* energy = getPsychoanalEnergy();
    float* threshold = getPsychoanalThreshold();
    
    juce::var thresholdV, energyV;
    
    for (int i = 0; i < 22; ++i) {
        thresholdV.append(rescalePsychoanal(threshold[i]));
        energyV.append(rescalePsychoanal(energy[i]));
    }
    
    auto psychoSpectrum = parameters.state.getChildWithName("psychoanal");
    psychoSpectrum.setProperty("threshold", thresholdV, nullptr);
    psychoSpectrum.setProperty("energy", energyV, nullptr);

    float* preBend = getMDCTpreBend();
    float* postBend = getMDCTpostBend();

    juce::var preBendV, postBendV;

    for (int i = 0; i < 576; ++i) {
        preBendV.append(rescaleMDCT(preBend[i]));
        postBendV.append(rescaleMDCT(postBend[i]));
    }
    auto mdctSamples = parameters.state.getChildWithName("mdct");
    mdctSamples.setProperty("pre", preBendV, nullptr);
    mdctSamples.setProperty("post",postBendV, nullptr);
}
