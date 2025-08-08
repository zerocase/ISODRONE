#pragma once
#include <JuceHeader.h>
#include "IsoSound.h"

// Human voice frequency ranges (approximate)
struct VoiceRanges {
    static constexpr float BASS_LOW = 87.31f;      // F2
    static constexpr float BASS_HIGH = 349.23f;    // F4
    static constexpr float BARITONE_LOW = 98.00f;  // G2
    static constexpr float BARITONE_HIGH = 392.00f; // G4
    static constexpr float TENOR_LOW = 130.81f;    // C3
    static constexpr float TENOR_HIGH = 523.25f;   // C5
    static constexpr float ALTO_LOW = 174.61f;     // F3
    static constexpr float ALTO_HIGH = 698.46f;    // F5
    static constexpr float SOPRANO_LOW = 261.63f;  // C4
    static constexpr float SOPRANO_HIGH = 1046.50f; // C6
    
    // Combined human voice range
    static constexpr float HUMAN_LOW = BASS_LOW;
    static constexpr float HUMAN_HIGH = SOPRANO_HIGH;
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
    
    // Voice range mapping methods
    void setVoiceType(VoiceType type) { voiceType = type; }
    VoiceType getVoiceType() const { return voiceType; }
    float mapMidiToHumanVoice(int midiNote);
    void setUseVoiceMapping(bool shouldUse) { useVoiceMapping = shouldUse; }
    
private:
    juce::ADSR adsr;
    juce::ADSR::Parameters adsrParams;
    juce::dsp::Oscillator<float> osc { [](float x) { return x / juce::MathConstants<float>::pi; }};
    juce::dsp::Gain<float> gain;
    bool isPrepared { false };
    VoiceType voiceType = FULL_RANGE;
    bool useVoiceMapping = true;
};