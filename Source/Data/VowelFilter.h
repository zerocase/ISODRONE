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
    enum VowelType
    {
        A = 0,
        E,
        I,
        O,
        U,
        NumVowels
    };

    VowelFilter();
    ~VowelFilter();

    // Setup and processing
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    // Main controls
    void setVowelType(VowelType vowel);
    void setFundamentalFrequency(float frequency);
    
    // Additional filter controls
    void setFormantShift(float shiftFactor);        // Scale all formants up/down (0.5 - 2.0)
    void setFormantSpread(float spreadFactor);      // Spread formants apart/together (0.5 - 2.0)
    void setBandwidthScale(float bandwidthFactor);  // Make formants narrower/wider (0.5 - 3.0)
    void setResonanceGain(float gainFactor);        // Overall formant intensity (0.1 - 2.0)
    void setHarmonicAlignment(bool enabled);        // Snap formants to harmonics
    
    // Parameter getters
    VowelType getVowelType() const { return currentVowel; }
    float getFundamentalFrequency() const { return currentFundamental; }
    float getFormantShift() const { return formantShift; }
    float getFormantSpread() const { return formantSpread; }
    float getBandwidthScale() const { return bandwidthScale; }
    float getResonanceGain() const { return resonanceGain; }
    bool getHarmonicAlignment() const { return harmonicAlignment; }

private:
    struct FormantData
    {
        float f1, f2, f3;           // Formant frequencies (Hz)
        float bw1, bw2, bw3;        // Bandwidths (Hz)
        float gain1, gain2, gain3;  // Gains (linear)
    };

    // Formant data for each vowel
    static const FormantData vowelFormants[NumVowels];

    // Filter bank - 3 formants per channel
    std::vector<std::unique_ptr<juce::IIRFilter>> formant1Filters;
    std::vector<std::unique_ptr<juce::IIRFilter>> formant2Filters;
    std::vector<std::unique_ptr<juce::IIRFilter>> formant3Filters;

    // Processing buffers
    juce::AudioBuffer<float> tempBuffer;

    // Audio parameters
    double sampleRate;
    int numChannels;

    // Core vowel parameters
    VowelType currentVowel;
    float currentFundamental;
    float referenceFundamental;     // Reference pitch for scaling (220Hz)

    // Advanced formant controls
    float formantShift;             // Global formant frequency scaling
    float formantSpread;            // Formant frequency spreading
    float bandwidthScale;           // Bandwidth scaling factor
    float resonanceGain;            // Overall formant gain
    bool harmonicAlignment;         // Snap formants to harmonics

    // Internal methods
    void updateFilters();
    void createBandpassFilter(juce::IIRFilter* filter, float frequency, float bandwidth, float gain);
    float findNearestHarmonic(float formantFreq, float fundamental);
    float applyFormantAdjustments(float baseFrequency, int formantIndex);
};