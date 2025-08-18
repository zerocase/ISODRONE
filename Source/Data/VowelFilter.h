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

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void process(juce::AudioBuffer<float>& buffer);
    void reset();

    // Parameter setters
    void setVowelType(VowelType vowel);
    VowelType getVowelType() const { return currentVowel; }

private:
    struct FormantData
    {
        float f1, f2, f3;        // Formant frequencies
        float bw1, bw2, bw3;     // Bandwidths
        float gain1, gain2, gain3; // Gains
    };

    // Formant data for each vowel
    static const FormantData vowelFormants[NumVowels];

    // Filter bank - 3 formants per channel
    std::vector<std::unique_ptr<juce::IIRFilter>> formant1Filters;
    std::vector<std::unique_ptr<juce::IIRFilter>> formant2Filters;
    std::vector<std::unique_ptr<juce::IIRFilter>> formant3Filters;
    
    // Parallel processing buffers
    juce::AudioBuffer<float> tempBuffer;
    
    // Audio parameters
    double sampleRate;
    int numChannels;
    
    // Current vowel
    VowelType currentVowel;
    
    void updateFilters();
    void createBandpassFilter(juce::IIRFilter* filter, float frequency, float bandwidth, float gain);
};