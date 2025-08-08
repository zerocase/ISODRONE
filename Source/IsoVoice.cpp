/*
 ==============================================================================
 IsoVoice.cpp
 Created: 7 Aug 2025 7:50:21pm
 Author: zerocase
 ==============================================================================
*/
#include "IsoVoice.h"

//==============================================================================
// NEW: GlottalOscillator Implementation
//==============================================================================

void GlottalOscillator::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    this->sampleRate = static_cast<float>(sampleRate);
    phaseIncrement = frequency / sampleRate;
    updateLFParameters();
    isPrepared = true;
}

void GlottalOscillator::setFrequency(float freq)
{
    frequency = freq;
    if (isPrepared)
    {
        phaseIncrement = frequency / sampleRate;
        updateLFParameters();
    }
}

void GlottalOscillator::setOpenQuotient(float oq)
{
    openQuotient = juce::jlimit(0.1f, 0.9f, oq);
    updateLFParameters();
}

void GlottalOscillator::setAsymmetryCoeff(float alpha)
{
    asymmetryCoeff = juce::jlimit(0.1f, 2.0f, alpha);
}

void GlottalOscillator::setBreathiness(float breath)
{
    breathiness = juce::jlimit(0.0f, 1.0f, breath);
}

void GlottalOscillator::setTenseness(float tension)
{
    tenseness = juce::jlimit(0.0f, 1.0f, tension);
}

float GlottalOscillator::getNextSample()
{
    if (!isPrepared) return 0.0f;
    
    // Generate LF pulse
    float glottalSample = generateLFPulse(phase);
    
    // Add breath noise component
    float breathNoise = generateBreathNoise();
    float output = glottalSample + (breathNoise * breathiness);
    
    // Update phase
    phase += phaseIncrement;
    if (phase >= 1.0f)
        phase -= 1.0f;
    
    return output * 1.0f; // Remove the scaling that was making it too quiet
}

void GlottalOscillator::reset()
{
    phase = 0.0f;
}

float GlottalOscillator::generateLFPulse(float t)
{
    // Simplified LF model implementation
    // t is normalized phase (0.0 to 1.0)
    
    if (t <= te) // Open phase
    {
        if (t <= tp) // Rising phase
        {
            // Sine-based rising edge
            float normalizedT = t / tp;
            return std::sin(juce::MathConstants<float>::pi * normalizedT) * tenseness;
        }
        else // Falling phase (tp to te)
        {
            float normalizedT = (t - tp) / (te - tp);
            // Exponential decay with asymmetry
            return tenseness * std::exp(-asymmetryCoeff * normalizedT * 5.0f);
        }
    }
    else // Closed phase (te to 1.0)
    {
        return 0.0f;
    }
}

float GlottalOscillator::generateBreathNoise()
{
    return (random.nextFloat() * 2.0f - 1.0f) * 0.1f; // Low-level noise
}

void GlottalOscillator::updateLFParameters()
{
    te = openQuotient;     // Normalized time when glottis closes
    tp = te * 0.4f;        // Peak occurs at 40% of open phase
}

//==============================================================================
// EXISTING IsoVoice Implementation with ADDITIONS
//==============================================================================

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
    
    // Set frequency for both oscillators
    osc.setFrequency(frequency);
    glottalOsc.setFrequency(frequency);  // NEW: Also set glottal oscillator frequency
    
    adsr.noteOn();
}

void IsoVoice::stopNote (float velocity, bool allowTailOff)
{
    adsr.noteOff();
}

void IsoVoice::controllerMoved (int controllerNumber, int newControllerValue)
{
    float normalizedValue = newControllerValue / 127.0f;
    
    // EXISTING CONTROLLERS
    if (controllerNumber == 71) // Filter resonance CC, repurposed for voice type
    {
        int typeIndex = juce::jlimit(0, 5, (newControllerValue * 5) / 127);
        setVoiceType(static_cast<VoiceType>(typeIndex));
    }
    else if (controllerNumber == 72) // Filter release CC, repurposed for mapping toggle
    {
        setUseVoiceMapping(newControllerValue >= 64);
    }
    // NEW CONTROLLERS for glottal oscillator
    else if (controllerNumber == 73) // Open quotient
    {
        glottalOsc.setOpenQuotient(0.3f + normalizedValue * 0.4f); // 0.3 to 0.7
    }
    else if (controllerNumber == 74) // Asymmetry coefficient  
    {
        glottalOsc.setAsymmetryCoeff(0.1f + normalizedValue * 1.9f); // 0.1 to 2.0
    }
    else if (controllerNumber == 75) // Breathiness
    {
        glottalOsc.setBreathiness(normalizedValue);
    }
    else if (controllerNumber == 76) // Tenseness
    {
        glottalOsc.setTenseness(normalizedValue);
    }
}

void IsoVoice::pitchWheelMoved (int newPitchWheelValue)
{
    // EXISTING CODE - no changes
    float pitchBend = (newPitchWheelValue - 8192) / 8192.0f;
    // You could implement pitch bend logic here if needed
}

void IsoVoice::prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels)
{
    // EXISTING CODE
    adsr.setSampleRate (sampleRate);
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = outputChannels;
    osc.prepare (spec);
    gain.prepare (spec);
    gain.setGainLinear (1.0f);
    
    // NEW: Prepare glottal oscillator
    glottalOsc.prepareToPlay(sampleRate, samplesPerBlock);
    
    isPrepared = true;
}

void IsoVoice::renderNextBlock (juce::AudioBuffer< float > &outputBuffer, int startSample, int numSamples)
{
    jassert (isPrepared);
    
    if (oscillatorType == 0) // Sawtooth oscillator
    {
        // EXISTING: Process primary oscillator
        juce::dsp::AudioBlock<float> audioBlock { outputBuffer, (size_t)startSample };
        osc.process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
        
        // Apply gain and ADSR
        gain.process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
        adsr.applyEnvelopeToBuffer (outputBuffer, startSample, numSamples);
    }
    else // Glottal oscillator
    {
        // Clear buffer first
        for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            outputBuffer.clear(channel, startSample, numSamples);
        
        // Generate glottal samples
        for (int sample = startSample; sample < startSample + numSamples; ++sample)
        {
            float glottalSample = glottalOsc.getNextSample();
            
            for (int channel = 0; channel < outputBuffer.getNumChannels(); ++channel)
            {
                outputBuffer.setSample(channel, sample, glottalSample);
            }
        }
        
        // Apply gain and ADSR
        juce::dsp::AudioBlock<float> audioBlock { outputBuffer, (size_t)startSample };
        gain.process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
        adsr.applyEnvelopeToBuffer (outputBuffer, startSample, numSamples);
    }
}

// NEW: Oscillator selection and glottal controls
void IsoVoice::setOscillatorType(int oscType)
{
    oscillatorType = juce::jlimit(0, 1, oscType);
}

void IsoVoice::setGlottalParams(float oq, float alpha, float breath, float tension)
{
    glottalOsc.setOpenQuotient(oq);
    glottalOsc.setAsymmetryCoeff(alpha);
    glottalOsc.setBreathiness(breath);
    glottalOsc.setTenseness(tension);
}