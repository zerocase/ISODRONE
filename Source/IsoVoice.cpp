/*
  ==============================================================================

    IsoVoice.cpp
    Created: 7 Aug 2025 7:50:21pm
    Author:  zerocase

  ==============================================================================
*/

#include "IsoVoice.h"

bool IsoVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<juce::SynthesiserSound*>(sound) != nullptr;
}

float IsoVoice::mapMidiToHumanVoice(int midiNote)
{
    // MIDI range is 0-127, we'll use the practical range of 21-108 (A0 to C8)
    float normalizedMidi = juce::jlimit(0.0f, 1.0f, (midiNote - 21) / 87.0f);
    
    float lowFreq, highFreq;
    
    switch (voiceType) {
        case BASS:
            lowFreq = VoiceRanges::BASS_LOW;
            highFreq = VoiceRanges::BASS_HIGH;
            break;
        case BARITONE:
            lowFreq = VoiceRanges::BARITONE_LOW;
            highFreq = VoiceRanges::BARITONE_HIGH;
            break;
        case TENOR:
            lowFreq = VoiceRanges::TENOR_LOW;
            highFreq = VoiceRanges::TENOR_HIGH;
            break;
        case ALTO:
            lowFreq = VoiceRanges::ALTO_LOW;
            highFreq = VoiceRanges::ALTO_HIGH;
            break;
        case SOPRANO:
            lowFreq = VoiceRanges::SOPRANO_LOW;
            highFreq = VoiceRanges::SOPRANO_HIGH;
            break;
        case FULL_RANGE:
        default:
            lowFreq = VoiceRanges::HUMAN_LOW;
            highFreq = VoiceRanges::HUMAN_HIGH;
            break;
    }
    
    // Logarithmic interpolation for more musical scaling
    float logLow = std::log(lowFreq);
    float logHigh = std::log(highFreq);
    float logFreq = logLow + normalizedMidi * (logHigh - logLow);
    
    return std::exp(logFreq);
}

void IsoVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound *sound, int currentPitchWheelPosition)
{
    float frequency;
    
    if (useVoiceMapping) {
        // Use human voice mapping
        frequency = mapMidiToHumanVoice(midiNoteNumber);
    } else {
        // Use standard MIDI conversion
        frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    }
    
    osc.setFrequency(frequency);
    adsr.noteOn();
}

void IsoVoice::stopNote (float velocity, bool allowTailOff)
{
    adsr.noteOff();
}

void IsoVoice::controllerMoved (int controllerNumber, int newControllerValue)
{
    // Voice type switching via MIDI CC
    if (controllerNumber == 71) // Filter resonance CC, repurposed for voice type
    {
        int typeIndex = juce::jlimit(0, 5, (newControllerValue * 5) / 127);
        setVoiceType(static_cast<VoiceType>(typeIndex));
    }
    // Toggle voice mapping on/off
    else if (controllerNumber == 72) // Filter release CC, repurposed for mapping toggle
    {
        setUseVoiceMapping(newControllerValue >= 64);
    }
}

void IsoVoice::pitchWheelMoved (int newPitchWheelValue)
{
    // Standard pitch wheel implementation - could be enhanced to respect voice ranges
    float pitchBend = (newPitchWheelValue - 8192) / 8192.0f; // Normalize to -1 to +1
    // You could implement pitch bend logic here if needed
}

void IsoVoice::prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels)
{
    adsr.setSampleRate (sampleRate);
    
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = outputChannels;
    
    osc.prepare (spec);
    gain.prepare (spec);
    gain.setGainLinear (0.5f);
    
    isPrepared = true;
}

void IsoVoice::renderNextBlock (juce::AudioBuffer< float > &outputBuffer, int startSample, int numSamples)
{
    jassert (isPrepared);
    
    juce::dsp::AudioBlock<float> audioBlock { outputBuffer, (size_t)startSample };
    osc.process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
    gain.process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
    adsr.applyEnvelopeToBuffer (outputBuffer, startSample, numSamples);
}