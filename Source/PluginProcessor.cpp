/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ISODRONEAudioProcessor::ISODRONEAudioProcessor()
    : AudioProcessor (
#ifndef JucePlugin_PreferredChannelConfigurations
                     BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
#endif
                     ),
        apvts (*this, nullptr, "Parameters", createParams())
{
    iso.addSound (new IsoSound());
    iso.addVoice (new IsoVoice());
}

ISODRONEAudioProcessor::~ISODRONEAudioProcessor()
{
}

//==============================================================================
const juce::String ISODRONEAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ISODRONEAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ISODRONEAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ISODRONEAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ISODRONEAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ISODRONEAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ISODRONEAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ISODRONEAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ISODRONEAudioProcessor::getProgramName (int index)
{
    return {};
}

void ISODRONEAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ISODRONEAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    iso.setCurrentPlaybackSampleRate (sampleRate);
    for (int i = 0; i < iso.getNumVoices(); i++)
    {
        if (auto voice = dynamic_cast<IsoVoice*>(iso.getVoice(i)))
        {
            voice->prepareToPlay (sampleRate, samplesPerBlock, getTotalNumOutputChannels());
        }
    }

    // Start a continuous note
    //iso.noteOn(1, 30, 1.0f); // channel 1, middle C, full velocity
}

void ISODRONEAudioProcessor::releaseResources()
{   
    for (int i = 0; i < iso.getNumVoices(); i++)
    {
        if (auto voice = dynamic_cast<IsoVoice*>(iso.getVoice(i)))
        {
            voice->reset_filter();
        }
    }
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ISODRONEAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void ISODRONEAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Get oscillator type and glottal parameters
    auto& oscWaveChoice = *apvts.getRawParameterValue("OSC1WAVETYPE");

    // Only debug oscillator changes when they actually occur
    static int lastOscChoice = -1;
    int currentOscChoice = static_cast<int>(oscWaveChoice.load());
    
    if (currentOscChoice != lastOscChoice) {
        DBG("ProcessBlock: Oscillator changed to " + 
            juce::String(currentOscChoice == 0 ? "SAWTOOTH" : "GLOTTAL"));
        lastOscChoice = currentOscChoice;
    }

    auto& openQuotient = *apvts.getRawParameterValue("OPENQUOT");
    auto& asymmetry = *apvts.getRawParameterValue("ASYMMETRY");
    auto& breathiness = *apvts.getRawParameterValue("BREATHINESS");
    auto& tenseness = *apvts.getRawParameterValue("TENSENESS");

    auto& attack = *apvts.getRawParameterValue("ATTACK");
    auto& decay = *apvts.getRawParameterValue("DECAY");
    auto& sustain = *apvts.getRawParameterValue("SUSTAIN");
    auto& release = *apvts.getRawParameterValue("RELEASE");

    for (int i = 0; i < iso.getNumVoices(); ++i)
    {
        if (auto voice = dynamic_cast<IsoVoice*>(iso.getVoice(i)))
        {
            // Set oscillator type
            voice->getOscillator().setWaveType(currentOscChoice);
            
            // Set glottal parameters
            voice->setGlottalParams(openQuotient.load(), asymmetry.load(), breathiness.load(), tenseness.load());
            
            // Update ADSR
            voice->update(attack.load(), decay.load(), sustain.load(), release.load());
        }
    }
    
    for (const juce::MidiMessageMetadata metadata : midiMessages)
        if (metadata.numBytes == 3)
            juce::Logger::writeToLog ("TimeStamp: " + juce::String (metadata.getMessage().getTimeStamp()));
    midiProcessor.process(midiMessages);
    iso.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool ISODRONEAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* ISODRONEAudioProcessor::createEditor()
{
    return new ISODRONEAudioProcessorEditor (*this);
}

//==============================================================================
void ISODRONEAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ISODRONEAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ISODRONEAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout ISODRONEAudioProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // ADSR parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat> ("ATTACK", "Attack", juce::NormalisableRange<float> {0.1f, 1.0f}, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat> ("DECAY", "Decay", juce::NormalisableRange<float> {0.1f, 1.0f}, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat> ("SUSTAIN", "Sustain", juce::NormalisableRange<float> {0.1f, 1.0f}, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat> ("RELEASE", "Release", juce::NormalisableRange<float> {0.1f, 3.0f}, 0.4f));
    
    // Oscillator type parameter - Default is index 1 (Glottal)
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("OSC1WAVETYPE", "Osc 1 Wave Type", juce::StringArray { "Sawtooth", "Glottal" }, 1));

    // Glottal oscillator parameters
    params.push_back(std::make_unique<juce::AudioParameterFloat> ("OPENQUOT", "Open Quotient", juce::NormalisableRange<float> {0.3f, 0.7f}, 0.6f));
    params.push_back(std::make_unique<juce::AudioParameterFloat> ("ASYMMETRY", "Asymmetry", juce::NormalisableRange<float> {0.1f, 2.0f}, 0.7f));
    params.push_back(std::make_unique<juce::AudioParameterFloat> ("BREATHINESS", "Breathiness", juce::NormalisableRange<float> {0.0f, 1.0f}, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterFloat> ("TENSENESS", "Tenseness", juce::NormalisableRange<float> {0.0f, 1.0f}, 0.8f));

    //Filter
    params.push_back (std::make_unique<juce::AudioParameterChoice> ("FILTERTYPE", "Filter Type", juce::StringArray { "Low-Pass", "Band-Pass", "High-Pass" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat> ("FILTERCUTOFF", "Filter Cutoff", juce::NormalisableRange<float> {20.0f, 20000.0f, 0.1f, 0.6f}, 200.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat> ("FILTERRES", "Filter Resonance", juce::NormalisableRange<float> {1.0f, 10.0f, 0.1f}, 1.0f));
    return { params.begin(), params.end() };
}