/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
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
    parameters(*this, nullptr, juce::Identifier("Maim"),
               {
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"butterfly_uu", 1},
                                                    "MDCT Butterfly up in up",
                                                    -1.0f,
                                                    2.0f,
                                                    1.0f),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"butterfly_ud", 1},
                                                    "MDCT Butterfly up in donwn",
                                                    -1.0f,
                                                    2.0f,
                                                    0.0f),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"butterfly_du", 1},
                                                    "MDCT Butterfly down in up",
                                                    -1.0f,
                                                    2.0f,
                                                    0.0f),
        std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"butterfly_dd", 1},
                                                    "MDCT Butterfly down in down",
                                                    -1.0f,
                                                    2.0f,
                                                    1.0f)
    })
{
    parameters.addParameterListener("butterfly_uu", this);
    parameters.addParameterListener("butterfly_ud", this);
    parameters.addParameterListener("butterfly_du", this);
    parameters.addParameterListener("butterfly_dd", this);
}

MaimAudioProcessor::~MaimAudioProcessor()
{
    parameters.removeParameterListener("butterfly_uu", this);
    parameters.removeParameterListener("butterfly_ud", this);
    parameters.removeParameterListener("butterfly_du", this);
    parameters.removeParameterListener("butterfly_dd", this);}

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
void MaimAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    lameController.init(sampleRate, samplesPerBlock, 128);
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
    lameController.setButterflyBends(
        ((juce::AudioParameterFloat*) parameters.getParameter("butterfly_uu"))->get(),
        ((juce::AudioParameterFloat*) parameters.getParameter("butterfly_ud"))->get(),
        ((juce::AudioParameterFloat*) parameters.getParameter("butterfly_du"))->get(),
        ((juce::AudioParameterFloat*) parameters.getParameter("butterfly_dd"))->get()
    );
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

    float* samplesL;
    float* samplesR;

    if (buffer.getNumChannels() == 2) {
        samplesL = buffer.getWritePointer(0);
        samplesR = buffer.getWritePointer(1);
        lameController.addNextInput(samplesL, samplesR, buffer.getNumSamples());
        lameController.copyOutput(samplesL, samplesR, buffer.getNumSamples());
    }
    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
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
