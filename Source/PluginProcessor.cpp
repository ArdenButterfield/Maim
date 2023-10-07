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
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"drive", 1}, "drive", -36.f, 36.f, 0.f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"makeupgain", 1}, "makeup gain", -36.f, 36.f, 0.f));
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"butterflystandard", 1}, "MDCT Butterfly standard", 0.0f, 1.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"butterflycrossed", 1}, "MDCT Butterfly crossed", 0.0f, 1.0f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID {"mdctstep", 1},"MDCT band step", 8, 18, 18));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID {"mdctinvert", 1}, "MDCT band invert", false));
    parameters.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID {"mdctposthshift", 1}, "MDCT pitch shift", -100, 100, 0));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"mdctpostvshift", 1}, "MDCT amplitude shift", -1.f, 1.f, 0.f));
    parameters.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID {"mdctwindowincr", 1}, "MDCT window increment", -64, 64, 64));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"mdctfeedback", 1}, "MDCT feedback", 0, 1, 0));
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"turbo", 1}, "Turbo", 0, 1, 0));
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"thresholdbias", 1}, "Threshold bias", -1, 1, 0));
    
    for (int i = 0; i < NUM_REASSIGNMENT_BANDS; ++i) {
        std::stringstream id, name;
        id << "bandorder" << i;
        name << "Band order " << i;
        parameters.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{id.str(), 1,},name.str(),0,31,i));
    }
    
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID {"bitrate", 1}, "Bitrate", juce::StringArray {
            "8", "16", "24", "32", "40", "48", "56", "64", "80", "96", "112", "128", "160", "192", "224", "256", "320"},
        10));
    
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID {"encoder", 1}, "Encoder", juce::StringArray {"Blade", "Lame"}, 1));
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"hicut", 1}, "High cut", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.30f), 18000.f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"locut", 1}, "Low cut", juce::NormalisableRange<float>(10.f, 20000.f, 1.f, 0.30f), 10.f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {"mix", 1}, "Mix", 0.f, 100.f, 100.f));

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
    parameters(*this, nullptr, juce::Identifier("Maim"), makeParameters()),
      mp3ControllerManager(parameters),
      dryWetMixer(std::max(BLADELATENCYSAMPLES, LAMELATENCYSAMPLES))
{
    std::cout << this << " constructor\n";
    oldPreGain = 1;
    oldPostGain = 1;

    dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
    
    addPsychoanalStateToParameters();
    
    parameters.addParameterListener("hicut", this);
    parameters.addParameterListener("locut", this);
    parameters.addParameterListener("drive", this);
    parameters.addParameterListener("makeupgain", this);
    parameters.addParameterListener("mix", this);
    parameters.addParameterListener("encoder", this);
}

MaimAudioProcessor::~MaimAudioProcessor()
{
    parameters.removeParameterListener("hicut", this);
    parameters.removeParameterListener("locut", this);
    parameters.removeParameterListener("drive", this);
    parameters.removeParameterListener("makeupgain", this);
    parameters.removeParameterListener("mix", this);
    parameters.removeParameterListener("encoder", this);
}

void MaimAudioProcessor::addMdctSamplesToParameters()
{
    auto mdctSampes = parameters.copyState().getChildWithName("mdct");
    if (mdctSampes.isValid()) {
        return;
    }

    juce::var pre, post;
    for (int i = 0; i < 576; ++i) {
        pre.append(0.f);
        post.append(0.f);
    }

    parameters.state.appendChild(juce::ValueTree(
                                      "mdct",
                                      {
                                          juce::NamedValueSet::NamedValue("pre", pre),
                                          juce::NamedValueSet::NamedValue("post", post),
                                      }),
        nullptr);

}

void MaimAudioProcessor::addPsychoanalStateToParameters()
{
    auto psychoanalState = parameters.copyState().getChildWithName("psychoanal");
    if (psychoanalState.isValid()) {
        return;
    }


    juce::var threshold, energy;
    for (int i = 0; i < 22; ++i) {
        threshold.append(0.f);
        energy.append(0.f);
    }

    parameters.state.appendChild(juce::ValueTree(
        "psychoanal",
        {
            juce::NamedValueSet::NamedValue("threshold", threshold),
            juce::NamedValueSet::NamedValue("energy", energy),
            juce::NamedValueSet::NamedValue("shortblockindicator", juce::var(false))
        }),
        nullptr);
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
    std::cout << this << " prepare to play\n";

    setLatencySamples(currentLatencySamples());
    dryWetMixer.prepare({fs, static_cast<uint32_t>(samplesPerBlock), 2});
    sampleRate = fs;
    estimatedSamplesPerBlock = samplesPerBlock;
    int bitrate = MP3ControllerManager::bitrates[((juce::AudioParameterChoice*) parameters.getParameter("bitrate"))->getIndex()];
    mp3ControllerManager.initialize((int)fs, bitrate, samplesPerBlock);
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
    std::cout << this << " update params\n";

    auto hicut = ((juce::AudioParameterFloat*)parameters.getParameter("hicut"))->get();
    auto locut = ((juce::AudioParameterFloat*)parameters.getParameter("locut"))->get();
    hicut = std::max(1.f, std::min(hicut, (float)sampleRate / 2 - 1));
    locut = std::max(1.f, std::min(locut, (float)sampleRate / 2 - 1));
    for (auto &f: postFilterLo) {
        f.setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, hicut));
    }
    for (auto &f: postFilterHi) {
        f.setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, locut));
    }

    auto driveDB = ((juce::AudioParameterFloat*)parameters.getParameter("drive"))->get();
    auto makeupDB = ((juce::AudioParameterFloat*)parameters.getParameter("makeupgain"))->get();
    
    if (driveDB > 0) {
        makeupDB -= driveDB / 2;
    }

    dryWetMixer.setWetMixProportion(((juce::AudioParameterFloat*)parameters.getParameter("mix"))->get() / 100);

    preGain = juce::Decibels::decibelsToGain(driveDB);
    postGain = juce::Decibels::decibelsToGain(makeupDB);
    
    parametersNeedUpdating = false;
    setLatencySamples(currentLatencySamples());
}

void MaimAudioProcessor::processBlockStereo (juce::AudioBuffer<float>& buffer)
{
    dryWetMixer.pushDrySamples(buffer);

    if (oldPreGain != preGain) {
        buffer.applyGainRamp(0, buffer.getNumSamples(), oldPreGain, preGain);
        oldPreGain = preGain;
    } else {
        buffer.applyGain(preGain);
    }

    if (buffer.getNumSamples() <= estimatedSamplesPerBlock) {
        mp3ControllerManager.processBlock(buffer);
    }

    for (int i = 0; i < 2; ++i) {
        auto samples = buffer.getWritePointer(i);
        postFilterHi[i].processSamples(samples, buffer.getNumSamples());
        postFilterLo[i].processSamples(samples, buffer.getNumSamples());
    }

    if (oldPostGain != postGain) {
        buffer.applyGainRamp(0, buffer.getNumSamples(), oldPostGain, postGain);
        oldPostGain = postGain;
    } else {
        buffer.applyGain(postGain);
    }
    dryWetMixer.mixWetSamples(buffer);
}

void MaimAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                       juce::MidiBuffer& midiMessages)
{
    // std::cout << this << " process block\n";

    if (parametersNeedUpdating) {
        updateParameters();
    }
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = std::min(totalNumInputChannels,2); i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    if (totalNumInputChannels >= 2 && buffer.getNumChannels() >= 2) {
        processBlockStereo(buffer);
    } else if (totalNumInputChannels >= 1 && buffer.getNumChannels() >= 1) {
        juce::AudioBuffer<float> stereoBuffer(2, buffer.getNumSamples());
        stereoBuffer.copyFrom(0,0,buffer,0,0,buffer.getNumSamples());
        stereoBuffer.copyFrom(1,0,buffer,0,0,buffer.getNumSamples());
        processBlockStereo(stereoBuffer);
        buffer.copyFrom(0,0,stereoBuffer,0,0,buffer.getNumSamples());
    }
}

//==============================================================================
bool MaimAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* MaimAudioProcessor::createEditor()
{
    return new MaimAudioProcessorEditor (*this);
}

//==============================================================================
void MaimAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    std::cout << this << " get state\n";

    auto state = parameters.copyState();
    for (const juce::String parameterName : {"psychoanal", "mdct"}) {
        auto s = state.getChildWithName(parameterName);
        if (s.isValid()) {
            state.removeChild(s, nullptr);
        }
    }
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
    addPsychoanalStateToParameters();
    addMdctSamplesToParameters();

}
void MaimAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::cout << this << " set state\n";

    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr) {
        if (xmlState->hasTagName (parameters.state.getType())) {
            parameters.replaceState (juce::ValueTree::fromXml (*xmlState));
            addPsychoanalStateToParameters();
            addMdctSamplesToParameters();
        }
    }
}
int MaimAudioProcessor::currentLatencySamples()
{
    /* Currently, the number of samples of latency is determined experimentally,
     * using pluginval and an impulse response testing program. These experiments showed that
     * the impulse response held consistent across samplerates, bitrates, and block sizes; however,
     * Blade had an impulse response of silence at low bitrates, so some other response may be
     * needed for a more reliable test. I would like to try to get these numbers lower, which
     * I suspect may be possible, and also to have an actual proof of the latency, rather than
     * determining it experimentally.
     */
    auto encoder = (Encoder)((juce::AudioParameterChoice*)
                                  parameters.getParameter("encoder"))->getIndex();
    int latencySamples;
    if (encoder == 0) {
        // Blade
        latencySamples = BLADELATENCYSAMPLES;
    }
    if (encoder == 1) {
        // lame
        latencySamples = LAMELATENCYSAMPLES;
    }
    dryWetMixer.setWetLatency(latencySamples);

    return latencySamples;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MaimAudioProcessor();
}
