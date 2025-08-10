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

float IsoVoice::mapMidiToHumanVoice(int midiNote)
{
    // Define the MIDI range we want to use (typically C1 to C7 for voice synthesis)
    const int MIN_MIDI = 24;  // C1 (32.7 Hz)
    const int MAX_MIDI = 96;  // C7 (2093 Hz)
    
    // Clamp the input MIDI note to our usable range
    int clampedMidi = juce::jlimit(MIN_MIDI, MAX_MIDI, midiNote);
    
    // Normalize to 0.0 - 1.0 range
    float normalizedMidi = static_cast<float>(clampedMidi - MIN_MIDI) / (MAX_MIDI - MIN_MIDI);
    
    // Get frequency range for the selected voice type
    float lowFreq, highFreq;
    switch (voiceType) {
    case BASS:
        lowFreq = VoiceRanges::BASS_LOW;      // ~87 Hz (F2)
        highFreq = VoiceRanges::BASS_HIGH;    // ~349 Hz (F4)
        break;
    case BARITONE:
        lowFreq = VoiceRanges::BARITONE_LOW;  // ~98 Hz (G2)
        highFreq = VoiceRanges::BARITONE_HIGH;// ~392 Hz (G4)
        break;
    case TENOR:
        lowFreq = VoiceRanges::TENOR_LOW;     // ~131 Hz (C3)
        highFreq = VoiceRanges::TENOR_HIGH;   // ~523 Hz (C5)
        break;
    case ALTO:
        lowFreq = VoiceRanges::ALTO_LOW;      // ~175 Hz (F3)
        highFreq = VoiceRanges::ALTO_HIGH;    // ~698 Hz (F5)
        break;
    case SOPRANO:
        lowFreq = VoiceRanges::SOPRANO_LOW;   // ~262 Hz (C4)
        highFreq = VoiceRanges::SOPRANO_HIGH; // ~1047 Hz (C6)
        break;
    case FULL_RANGE:
    default:
        lowFreq = VoiceRanges::HUMAN_LOW;     // Full human vocal range
        highFreq = VoiceRanges::HUMAN_HIGH;
        break;
    }
    
    // Use logarithmic mapping for musical intervals
    float logLow = std::log(lowFreq);
    float logHigh = std::log(highFreq);
    float logFreq = logLow + normalizedMidi * (logHigh - logLow);
    
    return std::exp(logFreq);
}

void IsoVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound *sound, int currentPitchWheelPosition)
{
    float frequency;
    if (useVoiceMapping) {
        frequency = mapMidiToHumanVoice(midiNoteNumber);
    } else {
        frequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    }
    
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
    filterData.prepareToPlay(sampleRate, samplesPerBlock, outputChannels);

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