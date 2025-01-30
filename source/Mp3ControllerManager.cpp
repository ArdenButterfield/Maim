/*
  ==============================================================================

    Mp3ControllerManager.cpp
    Created: 10 Apr 2023 5:05:43pm
    Author:  Arden Butterfield

  ==============================================================================
*/

#include "Mp3ControllerManager.h"
#include "parameterIds.h"

void fadeTowards(float* currentBuffer, float* newBuffer, int numSamples, int startFadeFrom) {
    // NOTE: should potentially be an equal-power crossfade, to prevent a dip in volume.
    // Juce applyGainRamp uses a linear ramp as well, though.
    for (int i = startFadeFrom; i < numSamples; ++i) {
        float inc = (float)i / (float)(numSamples - startFadeFrom);
        currentBuffer[i] = inc * newBuffer[i] + (1 - inc) * currentBuffer[i];
    }
}


Mp3ControllerManager::Mp3ControllerManager(juce::AudioProcessorValueTreeState& p) :
    parameters(p),
    currentEncoder(lame),
    currentControllerIndex(0)
{
    parametersNeedUpdating = false;
    
    parameters.addParameterListener(BUTTERFLY_STANDARD_PARAM_ID, this);
    parameters.addParameterListener(BUTTERFLY_CROSSED_PARAM_ID, this);
    parameters.addParameterListener(MDCT_STEP_PARAM_ID, this);
    parameters.addParameterListener(MDCT_INVERT_PARAM_ID, this);
    parameters.addParameterListener(MDCT_PITCH_SHIFT_PARAM_ID, this);
    parameters.addParameterListener(MDCT_AMPLITUDE_SHIFT_PARAM_ID, this);
    parameters.addParameterListener(MDCT_WINDOW_INCREMENT_PARAM_ID, this);
    parameters.addParameterListener(BITRATE_PARAM_ID, this);
    parameters.addParameterListener(TURBO_PARAM_ID, this);
    parameters.addParameterListener(THRESHOLD_BIAS_PARAM_ID, this);
    parameters.addParameterListener(MDCT_FEEDBACK_PARAM_ID, this);
    parameters.addParameterListener(ENCODER_PARAM_ID, this);

    for (int i = 0; i < NUM_REASSIGNMENT_BANDS; ++i) {
        parameters.addParameterListener(BAND_ORDER_PARAM_IDS[i], this);
        bandReassignmentParameters[i] = (juce::AudioParameterInt*)parameters.getParameter(BAND_ORDER_PARAM_IDS[i]);
    }
}

Mp3ControllerManager::~Mp3ControllerManager()
{
    parameters.removeParameterListener(BUTTERFLY_STANDARD_PARAM_ID, this);
    parameters.removeParameterListener(BUTTERFLY_CROSSED_PARAM_ID, this);
    parameters.removeParameterListener(MDCT_STEP_PARAM_ID, this);
    parameters.removeParameterListener(MDCT_INVERT_PARAM_ID, this);
    parameters.removeParameterListener(MDCT_PITCH_SHIFT_PARAM_ID, this);
    parameters.removeParameterListener(MDCT_AMPLITUDE_SHIFT_PARAM_ID, this);
    parameters.removeParameterListener(MDCT_WINDOW_INCREMENT_PARAM_ID, this);
    parameters.removeParameterListener(BITRATE_PARAM_ID, this);
    parameters.removeParameterListener(TURBO_PARAM_ID, this);
    parameters.removeParameterListener(THRESHOLD_BIAS_PARAM_ID, this);
    parameters.removeParameterListener(MDCT_FEEDBACK_PARAM_ID, this);
    parameters.removeParameterListener(ENCODER_PARAM_ID, this);

    for (int i = 0; i < NUM_REASSIGNMENT_BANDS; ++i) {
        parameters.removeParameterListener(BAND_ORDER_PARAM_IDS[i], this);
    }
}

void Mp3ControllerManager::initialize (int _samplerate, int _initialBitrate, int _samplesPerBlock)
{
    std::memset(previousFrames, 0, 2 * 2 * 1152 * sizeof(float));
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

void Mp3ControllerManager::parameterChanged (const juce::String &parameterID, float newValue)
{
    parametersNeedUpdating = true;
}

void Mp3ControllerManager::changeController(int bitrate, Encoder encoder)
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
    } else if (encoder == blade) {
        desiredEncoder = blade;
        offController = &(bladeControllers[offIndex]);
    } else if (encoder == opus) {
        desiredEncoder = opus;
    }
    if (offController) {
        offController->init(samplerate, samplesPerBlock, desiredBitrate);
        wantingToSwitch = true;
    }
}

void Mp3ControllerManager::processBlock(juce::AudioBuffer<float>& buffer)
{
    // if (parametersNeedUpdating) {
    //     updateParameters();
    // }
    if (desiredEncoder == opus) {
        return;
    }
    updateParameters();
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
            offController->processFrame(previousFrames[0][0], previousFrames[0][1], nullptr, nullptr);
            offController->processFrame(previousFrames[1][0], previousFrames[1][1], nullptr, nullptr);
            offController->processFrame(frameIn[0], frameIn[1], frameOutNew[0], frameOutNew[1]);
            currentController->processFrame(frameIn[0], frameIn[1], frameOut[0], frameOut[1]);

            fadeTowards (frameOut[0], frameOutNew[0], MP3FRAMESIZE, 0);
            fadeTowards(frameOut[1], frameOutNew[1], MP3FRAMESIZE, 0);

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
        std::memcpy(previousFrames[0], previousFrames[1], 2 * MP3FRAMESIZE * sizeof(float));
        std::memcpy(previousFrames[1], frameIn, 2 * MP3FRAMESIZE * sizeof(float));
    }

    for (auto s = 0; s < buffer.getNumSamples(); ++s) {
        samplesL[s] = outputBufferL->dequeue();
    }
    for (auto s = 0; s < buffer.getNumSamples(); ++s) {
        samplesR[s] = outputBufferR->dequeue();
    }
}

void Mp3ControllerManager::updateParameters()
{
    auto encoder = (Encoder)((juce::AudioParameterChoice*)
                                parameters.getParameter(ENCODER_PARAM_ID))->getIndex();
    int bitrate = bitrates[((juce::AudioParameterChoice*)
                            parameters.getParameter(BITRATE_PARAM_ID))->getIndex()];
    changeController(bitrate, encoder);

    for (auto controller : {offController, currentController}) {
        if (controller == nullptr) {
            continue;
        }
        controller->setButterflyBends(
            ((juce::AudioParameterFloat*) parameters.getParameter(BUTTERFLY_STANDARD_PARAM_ID))->get(),
            ((juce::AudioParameterFloat*) parameters.getParameter(BUTTERFLY_CROSSED_PARAM_ID))->get(),
            ((juce::AudioParameterFloat*) parameters.getParameter(BUTTERFLY_CROSSED_PARAM_ID))->get(),
            ((juce::AudioParameterFloat*) parameters.getParameter(BUTTERFLY_STANDARD_PARAM_ID))->get()
        );

        controller->setMDCTbandstepBends(
            ((juce::AudioParameterBool*) parameters.getParameter(MDCT_INVERT_PARAM_ID))->get(),
            ((juce::AudioParameterInt*) parameters.getParameter(MDCT_STEP_PARAM_ID))->get()
        );

        controller->setMDCTfeedback(
            ((juce::AudioParameterFloat*) parameters.getParameter(MDCT_FEEDBACK_PARAM_ID))->get()
        );

        controller->setMDCTpostshiftBends(
            ((juce::AudioParameterInt*) parameters.getParameter(MDCT_PITCH_SHIFT_PARAM_ID))->get(),
           ((juce::AudioParameterFloat*) parameters.getParameter(MDCT_AMPLITUDE_SHIFT_PARAM_ID))->get()
        );
        controller->setMDCTwindowincrBends(
            ((juce::AudioParameterInt*) parameters.getParameter(MDCT_WINDOW_INCREMENT_PARAM_ID))->get()
        );
        controller->setBitrateSquishBends(
            ((juce::AudioParameterFloat*) parameters.getParameter(TURBO_PARAM_ID))->get()
        );

         controller->setThresholdBias(((juce::AudioParameterFloat*) parameters.getParameter(THRESHOLD_BIAS_PARAM_ID))->get()
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
    parametersNeedUpdating = false;
}

int Mp3ControllerManager::getBitrate()
{
    return currentBitrate;
}

float* Mp3ControllerManager::getPsychoanalEnergy()
{
    return currentController->getPsychoanalEnergy();
}

float* Mp3ControllerManager::getPsychoanalThreshold()
{
    return currentController->getPsychoanalThreshold();
}

float* Mp3ControllerManager::getMDCTpreBend()
{
    return currentController->getMDCTpreBend();
}

float* Mp3ControllerManager::getMDCTpostBend()
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

void Mp3ControllerManager::timerCallback()
{
    float* energy = getPsychoanalEnergy();
    float* threshold = getPsychoanalThreshold();
    
    juce::var thresholdV, energyV;

    if (energy) {
        for (int i = 0; i < 22; ++i) {
            energyV.append(rescalePsychoanal(energy[i]));
        }
    } else {
        for (int i = 0; i < 22; ++i) {
            energyV.append(0);
        }

    }

    if (threshold) {
        for (int i = 0; i < 22; ++i) {
            thresholdV.append(rescalePsychoanal(threshold[i]));
        }
    } else {
        for (int i = 0; i < 22; ++i) {
            thresholdV.append(0);
        }

    }

    auto psychoSpectrum = parameters.state.getChildWithName("psychoanal");
    psychoSpectrum.setProperty("threshold", thresholdV, nullptr);
    psychoSpectrum.setProperty("energy", energyV, nullptr);

    float* preBend = getMDCTpreBend();
    float* postBend = getMDCTpostBend();

    juce::var preBendV, postBendV;

    if (preBend) {
        for (int i = 0; i < 576; ++i) {
            preBendV.append (rescaleMDCT (preBend[i]));
        }
    } else {
        for (int i = 0; i < 576; ++i) {
            preBendV.append (0);
        }
    }
    if (postBend) {
        for (int i = 0; i < 576; ++i) {
            postBendV.append (rescaleMDCT (postBend[i]));
        }
    } else {
        for (int i = 0; i < 576; ++i) {
            postBendV.append (0);
        }
    }

    auto mdctSamples = parameters.state.getChildWithName("mdct");
    mdctSamples.setProperty("pre", preBendV, nullptr);
    mdctSamples.setProperty("post",postBendV, nullptr);
}
