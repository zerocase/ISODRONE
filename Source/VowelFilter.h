/*
  ==============================================================================

    VowelFilter.h
    Created: 8 Aug 2025 3:01:11pm
    Author:  zerocase

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class VowelFilter
{
public:
    VowelFilter();
    ~VowelFilter();

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& buffer);
    void reset();

private:
    // Static formant frequencies for "A" vowel (730Hz, 1090Hz, 2440Hz)
    static constexpr float FORMANT_1_FREQ = 730.0f;
    static constexpr float FORMANT_2_FREQ = 1090.0f;
    static constexpr float FORMANT_3_FREQ = 2440.0f;
    
    // Static Q values for each formant
    static constexpr float FORMANT_1_Q = 6.0f;
    static constexpr float FORMANT_2_Q = 8.0f;
    static constexpr float FORMANT_3_Q = 10.0f;
    
    // Static gain values (in linear, not dB)
    static constexpr float FORMANT_1_GAIN = 1.0f;    // 0 dB
    static constexpr float FORMANT_2_GAIN = 0.5f;    // -6 dB
    static constexpr float FORMANT_3_GAIN = 0.25f;   // -12 dB
    
    // Filter bank - 3 formants per channel
    std::vector<std::unique_ptr<juce::IIRFilter>> formant1Filters;
    std::vector<std::unique_ptr<juce::IIRFilter>> formant2Filters;
    std::vector<std::unique_ptr<juce::IIRFilter>> formant3Filters;
    
    // Parallel processing buffers
    juce::AudioBuffer<float> tempBuffer;
    
    // Audio parameters
    double sampleRate;
    int numChannels;
    
    void createBandpassFilter(juce::IIRFilter* filter, float frequency, float q, float gain);
};