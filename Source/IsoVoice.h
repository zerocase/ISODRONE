#pragma once
#include <JuceHeader.h>
#include "IsoSound.h"

// Human voice frequency ranges (existing - no changes)
struct VoiceRanges {
    static constexpr float BASS_LOW = 87.31f; // F2
    static constexpr float BASS_HIGH = 349.23f; // F4
    static constexpr float BARITONE_LOW = 98.00f; // G2
    static constexpr float BARITONE_HIGH = 392.00f; // G4
    static constexpr float TENOR_LOW = 130.81f; // C3
    static constexpr float TENOR_HIGH = 523.25f; // C5
    static constexpr float ALTO_LOW = 174.61f; // F3
    static constexpr float ALTO_HIGH = 698.46f; // F5
    static constexpr float SOPRANO_LOW = 261.63f; // C4
    static constexpr float SOPRANO_HIGH = 1046.50f; // C6
    
    // Combined human voice range
    static constexpr float HUMAN_LOW = BASS_LOW;
    static constexpr float HUMAN_HIGH = SOPRANO_HIGH;
};

// NEW: Simple Glottal Oscillator class
class GlottalOscillator
{
public:
    GlottalOscillator() = default;
    
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void setFrequency(float frequency);
    void setOpenQuotient(float oq);      // Controls how long glottis stays open (0.3-0.7)
    void setAsymmetryCoeff(float alpha); // Controls pulse asymmetry (0.1-2.0)
    void setBreathiness(float breath);   // Controls air noise component (0.0-1.0)
    void setTenseness(float tension);    // Controls vocal fold tension (0.0-1.0)
    
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
    float te = 0.0f;  // Time when flow returns to zero
    float tp = 0.0f;  // Time of peak flow
    
    juce::Random random;
    bool isPrepared = false;
};

class IsoVoice : public juce::SynthesiserVoice
{
public:
    enum VoiceType {
        BASS = 0,
        BARITONE,
        TENOR,
        ALTO,
        SOPRANO,
        FULL_RANGE
    };

    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound *sound, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void controllerMoved (int controllerNumber, int newControllerValue) override;
    void pitchWheelMoved (int newPitchWheelValue) override;
    void prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels);
    void renderNextBlock (juce::AudioBuffer< float > &outputBuffer, int startSample, int numSamples) override;

    // Voice range mapping methods (existing - no changes)
    void setVoiceType(VoiceType type) { voiceType = type; }
    VoiceType getVoiceType() const { return voiceType; }
    float mapMidiToHumanVoice(int midiNote);
    void setUseVoiceMapping(bool shouldUse) { useVoiceMapping = shouldUse; }
    
    // NEW: Oscillator selection and glottal controls
    void setOscillatorType(int oscType);     // 0 = sawtooth, 1 = glottal
    void setGlottalParams(float oq, float alpha, float breath, float tension);

private:
    // Existing members (no changes)
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    juce::dsp::Oscillator<float> osc { [](float x) { return x / juce::MathConstants<float>::pi; }};
    juce::dsp::Gain<float> gain;
    bool isPrepared { false };
    
    VoiceType voiceType = FULL_RANGE;
    bool useVoiceMapping = true;
    
    // NEW: Glottal oscillator and selection
    GlottalOscillator glottalOsc;
    int oscillatorType = 1;  // 0 = sawtooth, 1 = glottal
};