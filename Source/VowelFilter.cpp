/*
  ==============================================================================

    VowelFilter.cpp
    Created: 8 Aug 2025 3:01:11pm
    Author:  zerocase

  ==============================================================================
*/

#include "VowelFilter.h"

VowelFilter::VowelFilter()
    : sampleRate(44100.0)
    , numChannels(2)
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
    
    // Configure all filters with static "A" vowel formants
    for (int channel = 0; channel < numChannels; ++channel)
    {
        createBandpassFilter(formant1Filters[channel].get(), FORMANT_1_FREQ, FORMANT_1_BW, FORMANT_1_GAIN);
        createBandpassFilter(formant2Filters[channel].get(), FORMANT_2_FREQ, FORMANT_2_BW, FORMANT_2_GAIN);
        createBandpassFilter(formant3Filters[channel].get(), FORMANT_3_FREQ, FORMANT_3_BW, FORMANT_3_GAIN);
    }
    
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
            filteredSample *= 0.2f;
            
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