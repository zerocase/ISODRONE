/*
 ==============================================================================
 IsoVoice.h - Fixed to match OscData architecture
 ==============================================================================
*/
#pragma once
#include <JuceHeader.h>
#include "IsoSound.h"
#include "Data/ADSRData.h"
#include "Data/OscData.h"
#include "Data/VowelFilter.h"

// Forward declaration
class MidiProcessor;

class IsoVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound(juce::SynthesiserSound* sound) override;
    void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound *sound, int currentPitchWheelPosition) override;
    void stopNote(float velocity, bool allowTailOff) override;
    void controllerMoved(int controllerNumber, int newControllerValue) override;
    void pitchWheelMoved(int newPitchWheelValue) override;
    void prepareToPlay(double sampleRate, int samplesPerBlock, int outputChannels);
    void renderNextBlock(juce::AudioBuffer<float> &outputBuffer, int startSample, int numSamples) override;

    // ADSR control
    void update(const float attack, const float decay, const float sustain, const float release);

    // Oscillator access - now simplified since OscData handles oscillator types
    OscData& getOscillator() { return osc; }

    // Glottal parameter control (delegates to OscData)
    void setGlottalParams(float oq, float alpha, float breath, float tension);
    
    // VowelFilter control
    void setVowelType(VowelFilter::VowelType vowel) { filterData.setVowelType(vowel); }
    VowelFilter::VowelType getVowelType() const { return filterData.getVowelType(); }
    VowelFilter& getVowelFilter() { return filterData; }
    
    // MidiProcessor integration for pitch-aware filtering
    void setMidiProcessor(MidiProcessor* processor) { midiProcessor = processor; }
    
    void reset_filter();

private:
    VowelFilter filterData;
    ADSRData adsr;
    juce::AudioBuffer<float> isoBuffer;
    OscData osc; // Handles both sawtooth and glottal oscillators internally
    juce::dsp::Gain<float> gain;
    
    // Pitch tracking for vowel filter
    MidiProcessor* midiProcessor = nullptr;
    int currentMidiNote = -1;
    
    bool isPrepared { false };
    bool useVoiceMapping = true;
};