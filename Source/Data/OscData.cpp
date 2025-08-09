/*
  ==============================================================================

    OscData.cpp
    Created: 9 Aug 2025 3:36:54pm
    Author: zerocase

  ==============================================================================
*/

#include "OscData.h"

//==============================================================================
// GlottalOscillator Implementation
//==============================================================================

void GlottalOscillator::prepare(const juce::dsp::ProcessSpec& spec)
{
    this->sampleRate = static_cast<float>(spec.sampleRate);
    updateLFParameters();
    isPrepared = true;
}

void GlottalOscillator::setFrequency(float freq)
{
    frequency = freq;
    phaseIncrement = frequency / sampleRate;
}

void GlottalOscillator::setOpenQuotient(float oq)
{
    openQuotient = juce::jlimit(0.3f, 0.7f, oq);
    updateLFParameters();
}

void GlottalOscillator::setAsymmetryCoeff(float alpha)
{
    asymmetryCoeff = juce::jlimit(0.1f, 2.0f, alpha);
    updateLFParameters();
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

    float sample = generateLFPulse(phase);
    
    // Add breathiness (noise component)
    if (breathiness > 0.0f)
    {
        float noise = generateBreathNoise();
        sample = sample * (1.0f - breathiness) + noise * breathiness;
    }
    
    // Apply tenseness (affects amplitude and harmonics)
    sample *= (0.5f + 0.5f * tenseness);
    
    // Update phase
    phase += phaseIncrement;
    if (phase >= 1.0f) phase -= 1.0f;
    
    return sample;
}

void GlottalOscillator::reset()
{
    phase = 0.0f;
}

float GlottalOscillator::generateLFPulse(float phase)
{
    // Simplified LF model implementation
    if (phase < te)
    {
        // Rising portion
        float t_norm = phase / tp;
        return std::sin(juce::MathConstants<float>::pi * t_norm);
    }
    else if (phase < openQuotient)
    {
        // Falling portion
        float t_norm = (phase - tp) / (te - tp);
        return std::exp(-asymmetryCoeff * t_norm) * std::cos(juce::MathConstants<float>::pi * t_norm);
    }
    else
    {
        // Closed portion
        return 0.0f;
    }
}

float GlottalOscillator::generateBreathNoise()
{
    return random.nextFloat() * 2.0f - 1.0f; // White noise [-1, 1]
}

void GlottalOscillator::updateLFParameters()
{
    te = openQuotient * 0.7f; // Approximate relationship
    tp = te * 0.4f; // Peak occurs early in the open phase
}

//==============================================================================
// OscData Implementation
//==============================================================================

OscData::OscData()
{
    // Initialize sawtooth oscillator
    sawOsc.initialise([](float x) { return x / juce::MathConstants<float>::pi; });
}

void OscData::prepareToPlay(juce::dsp::ProcessSpec& spec)
{
    // Both oscillators use the same spec
    sawOsc.prepare(spec);
    glottalOsc.prepare(spec);
    isPrepared = true;
}

void OscData::setWaveType(const int choice)
{
    switch (choice)
    {
        case 0:
            currentOscType = SAWTOOTH;
            break;
        case 1:
            currentOscType = GLOTTAL;
            break;
        default:
            jassertfalse;
            break;
    }
}

void OscData::setWaveFrequency(float frequency)
{
    currentFrequency = frequency;
    
    sawOsc.setFrequency(currentFrequency);
    glottalOsc.setFrequency(currentFrequency);
}

void OscData::reset()
{
    sawOsc.reset();
    glottalOsc.reset();
}

void OscData::setGlottalParams(float openQuotient, float asymmetry, float breathiness, float tenseness)
{
    glottalOsc.setOpenQuotient(openQuotient);
    glottalOsc.setAsymmetryCoeff(asymmetry);
    glottalOsc.setBreathiness(breathiness);
    glottalOsc.setTenseness(tenseness);
}


void OscData::getNextAudioBlock(juce::dsp::AudioBlock<float>& block)
{
    if (!isPrepared) 
    {
        block.clear();
        return;
    }
    
    switch (currentOscType)
    {
        case SAWTOOTH:
            {
                // Use the JUCE oscillator's process method directly
                juce::dsp::ProcessContextReplacing<float> context(block);
                sawOsc.process(context);
            }
            break;
            
        case GLOTTAL:
            {
                // Process sample by sample, copy to all channels
                for (size_t sample = 0; sample < block.getNumSamples(); ++sample)
                {
                    float oscillatorSample = glottalOsc.getNextSample();
                    
                    for (size_t channel = 0; channel < block.getNumChannels(); ++channel)
                    {
                        block.setSample(channel, sample, oscillatorSample);
                    }
                }
            }
            break;
    }
}

