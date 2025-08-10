/*
  ==============================================================================

    VowelFilter.cpp
    Created: 8 Aug 2025 3:01:11pm
    Author:  zerocase

  ==============================================================================
*/

#include "VowelFilter.h"

// Formant data for each vowel (frequencies in Hz, bandwidths in Hz, gains linear)
const VowelFilter::FormantData VowelFilter::vowelFormants[NumVowels] = {
    // A vowel
    { 730.0f, 1090.0f, 2440.0f, 80.0f, 90.0f, 120.0f, 1.5f, 1.0f, 0.5f },
    // E vowel  
    { 530.0f, 1840.0f, 2480.0f, 80.0f, 90.0f, 120.0f, 1.5f, 1.2f, 0.6f },
    // I vowel
    { 270.0f, 2290.0f, 3010.0f, 40.0f, 90.0f, 120.0f, 1.2f, 1.5f, 0.8f },
    // O vowel
    { 570.0f, 840.0f, 2410.0f, 80.0f, 80.0f, 120.0f, 1.5f, 0.8f, 0.5f },
    // U vowel
    { 440.0f, 1020.0f, 2240.0f, 80.0f, 80.0f, 120.0f, 1.2f, 0.6f, 0.4f }
};

VowelFilter::VowelFilter()
    : sampleRate(44100.0)
    , numChannels(2)
    , currentVowel(E)
{
}

VowelFilter::~VowelFilter()
{
}

void VowelFilter::prepareToPlay(double newSampleRate, int samplesPerBlock)
{
    sampleRate = newSampleRate;
    
    // Assume stereo for now, will be updated in processBlock if needed
    if (numChannels == 0)
        numChannels = 2;
    
    // Clear and resize filter arrays
    formant1Filters.clear();
    formant2Filters.clear();
    formant3Filters.clear();
    
    // Create filters for each channel
    for (int channel = 0; channel < numChannels; ++channel)
    {
        formant1Filters.push_back(std::make_unique<juce::IIRFilter>());
        formant2Filters.push_back(std::make_unique<juce::IIRFilter>());
        formant3Filters.push_back(std::make_unique<juce::IIRFilter>());
    }
    
    // Configure filters with current vowel formants
    updateFilters();
    
    // Prepare temporary buffer for parallel processing
    tempBuffer.setSize(numChannels, samplesPerBlock, false, true, true);
}

void VowelFilter::processBlock(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();
    
    // Check if there's any input signal to process
    float maxInput = buffer.getMagnitude(0, 0, numSamples);
    if (maxInput < 1e-6f) {
        return; // No signal to process, avoid filter artifacts
    }
    
    // Update channel count if needed
    if (channels != numChannels)
    {
        numChannels = channels;
        prepareToPlay(sampleRate, numSamples);
    }
    
    // Ensure temp buffer is the right size
    tempBuffer.setSize(channels, numSamples, false, false, true);
    
    // Clear the output buffer - we'll replace it with the filtered result
    tempBuffer.clear();
    
    // Create temporary arrays for each formant processing
    juce::AudioBuffer<float> formant1Buffer(channels, numSamples);
    juce::AudioBuffer<float> formant2Buffer(channels, numSamples);
    juce::AudioBuffer<float> formant3Buffer(channels, numSamples);
    
    // Copy input to each formant buffer
    formant1Buffer.makeCopyOf(buffer, true);
    formant2Buffer.makeCopyOf(buffer, true);
    formant3Buffer.makeCopyOf(buffer, true);
    
    // Process each formant buffer
    for (int channel = 0; channel < channels; ++channel)
    {
        formant1Filters[channel]->processSamples(formant1Buffer.getWritePointer(channel), numSamples);
        formant2Filters[channel]->processSamples(formant2Buffer.getWritePointer(channel), numSamples);
        formant3Filters[channel]->processSamples(formant3Buffer.getWritePointer(channel), numSamples);
    }
    
    // Sum all formant outputs into the temp buffer
    for (int channel = 0; channel < channels; ++channel)
    {
        auto* outputData = tempBuffer.getWritePointer(channel);
        auto* formant1Data = formant1Buffer.getReadPointer(channel);
        auto* formant2Data = formant2Buffer.getReadPointer(channel);
        auto* formant3Data = formant3Buffer.getReadPointer(channel);
        
        for (int sample = 0; sample < numSamples; ++sample)
        {
            // Sum all formant outputs
            float filteredSample = formant1Data[sample] + formant2Data[sample] + formant3Data[sample];
            
            // Apply overall gain scaling (conservative scaling to prevent clipping)
            filteredSample *= 0.5f;
            
            // Soft clipping for safety
            filteredSample = juce::jlimit(-0.95f, 0.95f, filteredSample);
            
            outputData[sample] = filteredSample;
        }
    }
    
    // Copy the filtered result back to the main buffer
    buffer.makeCopyOf(tempBuffer, true);
}

void VowelFilter::reset()
{
    for (auto& filter : formant1Filters)
        if (filter) filter->reset();
        
    for (auto& filter : formant2Filters)
        if (filter) filter->reset();
        
    for (auto& filter : formant3Filters)
        if (filter) filter->reset();
}

void VowelFilter::setVowelType(VowelType vowel)
{
    if (currentVowel != vowel)
    {
        currentVowel = vowel;
        updateFilters();
    }
}

void VowelFilter::updateFilters()
{
    // Get current formant data
    const FormantData& formant = vowelFormants[currentVowel];
    
    // Update each formant filter for all channels
    for (int channel = 0; channel < static_cast<int>(formant1Filters.size()); ++channel)
    {
        if (formant1Filters[channel])
            createBandpassFilter(formant1Filters[channel].get(), formant.f1, formant.bw1, formant.gain1);
            
        if (formant2Filters[channel])
            createBandpassFilter(formant2Filters[channel].get(), formant.f2, formant.bw2, formant.gain2);
            
        if (formant3Filters[channel])
            createBandpassFilter(formant3Filters[channel].get(), formant.f3, formant.bw3, formant.gain3);
    }
}

void VowelFilter::createBandpassFilter(juce::IIRFilter* filter, float frequency, float bandwidth, float gain)
{
    // Ensure frequency is within valid range
    frequency = juce::jlimit(50.0f, static_cast<float>(sampleRate * 0.4), frequency);
    bandwidth = juce::jlimit(20.0f, frequency * 0.5f, bandwidth); // Clamp bandwidth
    
    // Convert bandwidth to Q factor: Q = frequency / bandwidth
    float q = frequency / bandwidth;
    q = juce::jlimit(0.5f, 20.0f, q); // Limit Q to stable range
    
    // Create bandpass filter coefficients
    auto coefficients = juce::IIRCoefficients::makeBandPass(sampleRate, frequency, q);
    
    // Apply a more conservative gain to prevent instability
    float safeGain = juce::jlimit(0.1f, 2.0f, gain);
    
    // Scale coefficients properly - only scale the numerator (b coefficients)
    coefficients.coefficients[0] *= safeGain; // b0
    coefficients.coefficients[1] *= safeGain; // b1  
    coefficients.coefficients[2] *= safeGain; // b2
    // Don't modify a1, a2 (denominator coefficients)
    
    filter->setCoefficients(coefficients);
}