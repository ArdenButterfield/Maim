/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include <array>
#include <atomic>
#include <string>

#include "MP3ControllerManager.h"

//==============================================================================

juce::AudioProcessorValueTreeState::ParameterLayout makeParameters();

class MaimAudioProcessor  : public juce::AudioProcessor,
                            public juce::AudioProcessorValueTreeState::Listener
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    MaimAudioProcessor();
    ~MaimAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState();
    void parameterChanged (const juce::String &parameterID, float newValue) override;
    
private:
    void addPsychoanalStateToParameters();
    void addMdctSamplesToParameters();
    int currentLatencySamples();
    float preGain;
    float postGain;
    
    float oldPreGain;
    float oldPostGain;
    
    double sampleRate;
    int estimatedSamplesPerBlock;
    
    void updateParameters();
    std::atomic<bool> parametersNeedUpdating;
    juce::AudioProcessorValueTreeState parameters;
    
    MP3ControllerManager mp3ControllerManager;
        
    std::array<juce::IIRFilter, 2> postFilterLo;
    std::array<juce::IIRFilter, 2> postFilterHi;

    const int BLADELATENCYSAMPLES = 2209;
    const int LAMELATENCYSAMPLES = 2880;
    juce::dsp::DryWetMixer<float> dryWetMixer;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MaimAudioProcessor)
};
