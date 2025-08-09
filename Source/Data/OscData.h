/*
 ==============================================================================
 OscData.h
 Created: 9 Aug 2025 3:36:54pm
 Author: zerocase
 ==============================================================================
*/
#pragma once
#include <JuceHeader.h>

// Glottal Oscillator class
class GlottalOscillator
{
public:
    GlottalOscillator() = default;
    
    void prepare(const juce::dsp::ProcessSpec& spec);
    void setFrequency(float frequency);
    void setOpenQuotient(float oq); // Controls how long glottis stays open (0.3-0.7)
    void setAsymmetryCoeff(float alpha); // Controls pulse asymmetry (0.1-2.0)
    void setBreathiness(float breath); // Controls air noise component (0.0-1.0)
    void setTenseness(float tension); // Controls vocal fold tension (0.0-1.0)
    
    float getNextSample();
    void reset();
    
private:
    float generateLFPulse(float phase);
    float generateBreathNoise();
    void updateLFParameters();
    
    float frequency = 440.0f;
    float sampleRate = 44100.0f;
    float phase = 0.0f;
    float phaseIncrement = 0.0f;
    
    // LF Model parameters
    float openQuotient = 0.6f;
    float asymmetryCoeff = 0.7f;
    float breathiness = 0.1f;
    float tenseness = 0.8f;
    
    // Derived parameters
    float te = 0.0f; // Time when flow returns to zero
    float tp = 0.0f; // Time of peak flow
    
    juce::Random random;
    bool isPrepared = false;
};

class OscData
{
public:
    enum OscType {
        SAWTOOTH = 0,
        GLOTTAL = 1
    };
    
    OscData();
    ~OscData() = default;
    
    // Setup and control
    void prepareToPlay(juce::dsp::ProcessSpec& spec);
    void setWaveType(const int choice);
    void getNextAudioBlock(juce::dsp::AudioBlock<float>& block);
    
    // Frequency setting
    void setWaveFrequency(float frequency);  // Single, clear method
    void reset();
    
    // Glottal parameters
    void setGlottalParams(float openQuotient, float asymmetry, float breathiness, float tenseness);
    void setOpenQuotient(float oq) { glottalOsc.setOpenQuotient(oq); }
    void setAsymmetry(float alpha) { glottalOsc.setAsymmetryCoeff(alpha); }
    void setBreathiness(float breath) { glottalOsc.setBreathiness(breath); }
    void setTenseness(float tension) { glottalOsc.setTenseness(tension); }
    
    
private:
    juce::dsp::Oscillator<float> sawOsc;
    GlottalOscillator glottalOsc;
    
    OscType currentOscType = GLOTTAL;
    bool isPrepared = false;
    float currentFrequency = 440.0f;
};