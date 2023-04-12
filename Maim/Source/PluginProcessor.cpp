/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout makeParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"butterflystandard", 1},
                                                                     "MDCT Butterfly standard",
                                                                     0.0f,
                                                                     1.0f,
                                                                     1.0f));
    parameters.push_back(        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"butterflycrossed", 1},
                                                                             "MDCT Butterfly crossed",
                                                                             0.0f,
                                                                             1.0f,
                                                                             0.0f));
    parameters.push_back(        std::make_unique<juce::AudioParameterInt>(juce::ParameterID {"mdctstep", 1},
                                                                           "MDCT band step",
                                                                           8,
                                                                           18,
                                                                           18));
    parameters.push_back(        std::make_unique<juce::AudioParameterBool>(juce::ParameterID {"mdctinvert", 1},
                                                                            "MDCT band invert",
                                                                            false));
    parameters.push_back(        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"lopass", 1},
                                                                             "Lowpass filter",
                                                                             100.f,
                                                                             20000.f,
                                                                             18000.f));
    parameters.push_back(        std::make_unique<juce::AudioParameterInt>(juce::ParameterID {"mdctposthshift", 1},
                                                                           "MDCT post pitch shift",
                                                                           -100,
                                                                           100,
                                                                           0));
    parameters.push_back(        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"mdctpostvshift", 1},
                                                                             "MDCT post amplitude shift",
                                                                             -1.f,
                                                                             1.f,
                                                                             0.f));
    parameters.push_back(        std::make_unique<juce::AudioParameterInt>(juce::ParameterID {"mdctwindowincr", 1},
                                                                           "MDCT window increment",
                                                                           -64,
                                                                           64,
                                                                           64));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
         juce::ParameterID {"bitratesquish", 1}, "Bitrate squish", 0, 1, 0));
    parameters.push_back(        std::make_unique<juce::AudioParameterChoice>(juce::ParameterID {"bitrate", 1},
                                                                              "Bitrate",
                                                                              juce::StringArray {"8", "16", "24", "32", "40", "48", "56", "64", "80", "96", "112", "128", "160", "192", "224", "256", "320"},
                                                                              10));
    for (int i = 0; i < NUM_REASSIGNMENT_BANDS; ++i) {
        std::stringstream id, name;
        id << "bandorder" << i;
        name << "Band order " << i;
        parameters.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{id.str(), 1,},name.str(),0,31,i));
    }
    return {parameters.begin(), parameters.end()};
}

MaimAudioProcessor::MaimAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
    parameters(*this, nullptr, juce::Identifier("Maim"), makeParameters())
{    
    parameters.addParameterListener("lopass", this);
}

MaimAudioProcessor::~MaimAudioProcessor()
{
    parameters.removeParameterListener("lopass", this);

}

//==============================================================================
const juce::String MaimAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

juce::AudioProcessorValueTreeState& MaimAudioProcessor::getValueTreeState()
{
    return parameters;
}

void MaimAudioProcessor::parameterChanged (const juce::String &parameterID, float newValue)
{
    parametersNeedUpdating = true;
}

bool MaimAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MaimAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MaimAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MaimAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MaimAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MaimAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MaimAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MaimAudioProcessor::getProgramName (int index)
{
    return {};
}

void MaimAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MaimAudioProcessor::prepareToPlay (double fs, int samplesPerBlock)
{
    sampleRate = fs;
    estimatedSamplesPerBlock = samplesPerBlock;
    int bitrate = LameControllerManager::bitrates[((juce::AudioParameterChoice*) parameters.getParameter("bitrate"))->getIndex()];
    lameControllerManager = std::make_unique<LameControllerManager>((int)fs, bitrate, samplesPerBlock, parameters);
    parametersNeedUpdating = true;
}

void MaimAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MaimAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MaimAudioProcessor::updateParameters()
{
    
    for (auto &f: postFilter) {
        f.setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, ((juce::AudioParameterFloat*)parameters.getParameter("lopass"))->get()));
    }
    
    parametersNeedUpdating = false;
}

void MaimAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer& midiMessages)
{
    if (parametersNeedUpdating) {
        updateParameters();
    }
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    lameControllerManager->processBlock(buffer);
    
    if (buffer.getNumChannels() == 2) {
        auto samplesL = buffer.getWritePointer(0);
        auto samplesR = buffer.getWritePointer(1);
        postFilter[0].processSamples(samplesL, buffer.getNumSamples());
        postFilter[1].processSamples(samplesR, buffer.getNumSamples());
    }
}

//==============================================================================
bool MaimAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MaimAudioProcessor::createEditor()
{
    return new MaimAudioProcessorEditor (*this);
}

//==============================================================================
void MaimAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MaimAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.state.getType()))
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MaimAudioProcessor();
}
