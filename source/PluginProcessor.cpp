/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "parameterIds.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout makeParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {DRIVE_PARAM_ID, 1}, "drive", -36.f, 36.f, 0.f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {MAKEUP_GAIN_PARAM_ID, 1}, "makeup gain", -36.f, 36.f, 0.f));
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {BUTTERFLY_STANDARD_PARAM_ID, 1}, "MDCT Butterfly standard", 0.0f, 1.0f, 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {BUTTERFLY_CROSSED_PARAM_ID, 1}, "MDCT Butterfly crossed", 0.0f, 1.0f, 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID {MDCT_STEP_PARAM_ID, 1},"MDCT band step", 8, 18, 18));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID {MDCT_INVERT_PARAM_ID, 1}, "MDCT band invert", false));
    parameters.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID {MDCT_PITCH_SHIFT_PARAM_ID, 1}, "MDCT pitch shift", -100, 100, 0));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {MDCT_AMPLITUDE_SHIFT_PARAM_ID, 1}, "MDCT amplitude shift", -1.f, 1.f, 0.f));
    parameters.push_back(std::make_unique<juce::AudioParameterInt>(
        juce::ParameterID {MDCT_WINDOW_INCREMENT_PARAM_ID, 1}, "MDCT window increment", -64, 64, 64));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {MDCT_FEEDBACK_PARAM_ID, 1}, "MDCT feedback", 0, 1, 0));
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {TURBO_PARAM_ID, 1}, "Turbo", 0, 1, 0));
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {THRESHOLD_BIAS_PARAM_ID, 1}, "Threshold bias", -1, 1, 0));
    
    for (int i = 0; i < NUM_REASSIGNMENT_BANDS; ++i) {
        std::stringstream id, name;
        name << "Band order " << i;
        parameters.push_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{BAND_ORDER_PARAM_IDS[i], 1,},name.str(),0,31,i));
    }
    
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID {BITRATE_PARAM_ID, 1}, "Bitrate", juce::StringArray {
            "8", "16", "24", "32", "40", "48", "56", "64", "80", "96", "112", "128", "160", "192", "224", "256", "320"},
        10));
    
    parameters.push_back(std::make_unique<juce::AudioParameterChoice>(
        juce::ParameterID {ENCODER_PARAM_ID, 1}, "Encoder", juce::StringArray {"Blade", "Lame", "Opus"}, 1));
    
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {HI_CUT_PARAM_ID, 1}, "High cut", juce::NormalisableRange<float>(20.f, 20000.f, 1.f, 0.30f), 18000.f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {LO_CUT_PARAM_ID, 1}, "Low cut", juce::NormalisableRange<float>(10.f, 20000.f, 1.f, 0.30f), 10.f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID {MIX_PARAM_ID, 1}, "Mix", juce::NormalisableRange<float>(0.f, 100.f, 0.1f), 100.f));

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
      codecControllerManager(parameters),
      dryWetMixer(std::max(BLADELATENCYSAMPLES, LAMELATENCYSAMPLES))
{
    oldPreGain = 1;
    oldPostGain = 1;

    dryWetMixer.setMixingRule(juce::dsp::DryWetMixingRule::linear);
    
    addPsychoanalStateToParameters();
    
    parameters.addParameterListener(HI_CUT_PARAM_ID, this);
    parameters.addParameterListener(LO_CUT_PARAM_ID, this);
    parameters.addParameterListener(DRIVE_PARAM_ID, this);
    parameters.addParameterListener(MAKEUP_GAIN_PARAM_ID, this);
    parameters.addParameterListener(MIX_PARAM_ID, this);
    parameters.addParameterListener(ENCODER_PARAM_ID, this);
}

MaimAudioProcessor::~MaimAudioProcessor()
{
    parameters.removeParameterListener(HI_CUT_PARAM_ID, this);
    parameters.removeParameterListener(LO_CUT_PARAM_ID, this);
    parameters.removeParameterListener(DRIVE_PARAM_ID, this);
    parameters.removeParameterListener(MAKEUP_GAIN_PARAM_ID, this);
    parameters.removeParameterListener(MIX_PARAM_ID, this);
    parameters.removeParameterListener(ENCODER_PARAM_ID, this);
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

    setLatencySamples(currentLatencySamples());
    dryWetMixer.prepare({fs, static_cast<uint32_t>(samplesPerBlock), 2});
    sampleRate = fs;
    estimatedSamplesPerBlock = samplesPerBlock;
    int bitrate = Mp3ControllerManager::bitrates[((juce::AudioParameterChoice*) parameters.getParameter("bitrate"))->getIndex()];
    codecControllerManager.initialize((int)fs, bitrate, samplesPerBlock);
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

    auto hicut = ((juce::AudioParameterFloat*)parameters.getParameter(HI_CUT_PARAM_ID))->get();
    auto locut = ((juce::AudioParameterFloat*)parameters.getParameter(LO_CUT_PARAM_ID))->get();
    hicut = std::max(1.f, std::min(hicut, (float)sampleRate / 2 - 1));
    locut = std::max(1.f, std::min(locut, (float)sampleRate / 2 - 1));
    for (auto &f: postFilterLo) {
        f.setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, hicut));
    }
    for (auto &f: postFilterHi) {
        f.setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, locut));
    }

    auto driveDB = ((juce::AudioParameterFloat*)parameters.getParameter(DRIVE_PARAM_ID))->get();
    auto makeupDB = ((juce::AudioParameterFloat*)parameters.getParameter(MAKEUP_GAIN_PARAM_ID))->get();
    
/*
    if (driveDB > 0) {
        makeupDB -= driveDB / 2;
    }
*/

    dryWetMixer.setWetMixProportion(((juce::AudioParameterFloat*)parameters.getParameter(MIX_PARAM_ID))->get() / 100);

    preGain = juce::Decibels::decibelsToGain(driveDB);
    postGain = juce::Decibels::decibelsToGain(makeupDB);
    
    parametersNeedUpdating = false;
    setLatencySamples(currentLatencySamples());
}

void MaimAudioProcessor::processBlockStereo (juce::AudioBuffer<float>& buffer)
{
    dryWetMixer.pushDrySamples(buffer);

    if (!juce::approximatelyEqual(oldPreGain, preGain)) {
        buffer.applyGainRamp(0, buffer.getNumSamples(), oldPreGain, preGain);
        oldPreGain = preGain;
    } else {
        buffer.applyGain(preGain);
    }

    if (buffer.getNumSamples() <= estimatedSamplesPerBlock) {
        codecControllerManager.processBlock(buffer);
    }

    for (unsigned i = 0; i < 2; ++i) {
        auto samples = buffer.getWritePointer(static_cast<int> (i));
        postFilterHi[i].processSamples(samples, buffer.getNumSamples());
        postFilterLo[i].processSamples(samples, buffer.getNumSamples());
    }

    if (!juce::approximatelyEqual(oldPostGain, postGain)) {
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
                                  parameters.getParameter(ENCODER_PARAM_ID))->getIndex();
    int latencySamples;
    if (encoder == 0) {
        // Blade
        latencySamples = BLADELATENCYSAMPLES;
    }
    if (encoder == 1) {
        // lame
        latencySamples = LAMELATENCYSAMPLES;
    } else {
        // opus
        latencySamples = 0; // TEST
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
