/*
 ==============================================================================
 IsoVoice.cpp - Fixed to work with OscData architecture
 Created: 7 Aug 2025 7:50:21pm
 Author: zerocase
 ==============================================================================
*/
#include "IsoVoice.h"

//==============================================================================
// IsoVoice Implementation
//==============================================================================

bool IsoVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<juce::SynthesiserSound*>(sound) != nullptr;
}



void IsoVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound *sound, int currentPitchWheelPosition)
{
    float frequency;
    frequency = 127.0f;
    
    // Set frequency directly
    osc.setWaveFrequency(frequency);
    adsr.noteOn();
}

void IsoVoice::stopNote (float velocity, bool allowTailOff)
{
    adsr.noteOff();

    if (! allowTailOff || ! adsr.isActive())
    {
        clearCurrentNote();
    }
}

void IsoVoice::controllerMoved (int controllerNumber, int newControllerValue)
{
}

void IsoVoice::pitchWheelMoved (int newPitchWheelValue)
{
    float pitchBend = (newPitchWheelValue - 8192) / 8192.0f;
    // You could implement pitch bend logic here if needed
}

void IsoVoice::prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels)
{
    adsr.setSampleRate (sampleRate);

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = outputChannels;
    
    // Use OscData's prepareToPlay method
    osc.prepareToPlay(spec);
    gain.prepare (spec);

    gain.setGainLinear (1.0f);
    filterData.prepareToPlay(sampleRate, samplesPerBlock);

    isPrepared = true;
}

void IsoVoice::update(const float attack, const float decay, const float sustain, const float release)
{
    adsr.updateADSR (attack, decay, sustain, release);
}

void IsoVoice::reset_filter()
{
    filterData.reset();
}

void IsoVoice::renderNextBlock (juce::AudioBuffer< float > &outputBuffer, int startSample, int numSamples)
{
    jassert (isPrepared);
    
    if (! isVoiceActive())
        return;
    
    // Set up temporary buffer for this voice
    isoBuffer.setSize (outputBuffer.getNumChannels(), numSamples, false, false, true);
    isoBuffer.clear();

    
    // Get audio block from buffer - this is the TAP pattern
    juce::dsp::AudioBlock<float> audioBlock { isoBuffer };
    
    // Process the oscillator - this calls your OscData::getNextAudioBlock
    osc.getNextAudioBlock (audioBlock);

    // Apply gain processing 
    gain.process (juce::dsp::ProcessContextReplacing<float> (audioBlock));

    //Apply the vowel
    filterData.process(isoBuffer);

    
    // Apply ADSR envelope to the processed buffer
    adsr.applyEnvelopeToBuffer(isoBuffer, 0, numSamples);
    
    // Add the voice's output to the main output buffer
    for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
    {
        outputBuffer.addFrom(channel, startSample, isoBuffer, channel, 0, numSamples);
    }
    
    // Check if voice should be stopped
    if (!adsr.isActive())
        clearCurrentNote();
}

// Glottal parameter control (delegates to OscData)
void IsoVoice::setGlottalParams(float oq, float alpha, float breath, float tension)
{
    osc.setGlottalParams(oq, alpha, breath, tension);
}