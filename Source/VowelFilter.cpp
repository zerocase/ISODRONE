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
        createBandpassFilter(formant1Filters[channel].get(), FORMANT_1_FREQ, FORMANT_1_Q, FORMANT_1_GAIN);
        createBandpassFilter(formant2Filters[channel].get(), FORMANT_2_FREQ, FORMANT_2_Q, FORMANT_2_GAIN);
        createBandpassFilter(formant3Filters[channel].get(), FORMANT_3_FREQ, FORMANT_3_Q, FORMANT_3_GAIN);
    }
    
    // Prepare temporary buffer for parallel processing
    tempBuffer.setSize(numChannels, samplesPerBlock, false, true, true);
}

void VowelFilter::processBlock(juce::AudioBuffer<float>& buffer)
{
    const int numSamples = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();
    
    // Update channel count if needed
    if (channels != numChannels)
    {
        numChannels = channels;
        prepareToPlay(sampleRate, numSamples);
    }
    
    // Ensure temp buffer is the right size
    tempBuffer.setSize(channels, numSamples, false, false, true);
    
    // Process each formant in parallel by adding their outputs
    tempBuffer.clear();
    
    // Process formant 1
    for (int channel = 0; channel < channels; ++channel)
    {
        // Copy input to temp buffer for this formant
        auto* tempData = tempBuffer.getWritePointer(channel);
        auto* inputData = buffer.getReadPointer(channel);
        
        // Copy input data
        for (int sample = 0; sample < numSamples; ++sample)
            tempData[sample] = inputData[sample];
        
        // Process through formant 1 filter
        formant1Filters[channel]->processSamples(tempData, numSamples);
    }
    
    // Add formant 1 output to buffer
    for (int channel = 0; channel < channels; ++channel)
    {
        buffer.addFrom(channel, 0, tempBuffer, channel, 0, numSamples);
    }
    
    // Process formant 2
    for (int channel = 0; channel < channels; ++channel)
    {
        auto* tempData = tempBuffer.getWritePointer(channel);
        auto* inputData = buffer.getReadPointer(channel);
        
        // Reset temp buffer with original input
        for (int sample = 0; sample < numSamples; ++sample)
            tempData[sample] = inputData[sample];
        
        // Process through formant 2 filter
        formant2Filters[channel]->processSamples(tempData, numSamples);
    }
    
    // Add formant 2 output to buffer
    for (int channel = 0; channel < channels; ++channel)
    {
        buffer.addFrom(channel, 0, tempBuffer, channel, 0, numSamples);
    }
    
    // Process formant 3
    for (int channel = 0; channel < channels; ++channel)
    {
        auto* tempData = tempBuffer.getWritePointer(channel);
        auto* inputData = buffer.getReadPointer(channel);
        
        // Reset temp buffer with original input
        for (int sample = 0; sample < numSamples; ++sample)
            tempData[sample] = inputData[sample];
        
        // Process through formant 3 filter
        formant3Filters[channel]->processSamples(tempData, numSamples);
    }
    
    // Add formant 3 output to buffer
    for (int channel = 0; channel < channels; ++channel)
    {
        buffer.addFrom(channel, 0, tempBuffer, channel, 0, numSamples);
    }
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

void VowelFilter::createBandpassFilter(juce::IIRFilter* filter, float frequency, float q, float gain)
{
    // Ensure frequency is within valid range
    frequency = juce::jlimit(20.0f, static_cast<float>(sampleRate * 0.45), frequency);
    
    // Create bandpass filter coefficients
    auto coefficients = juce::IIRCoefficients::makeBandPass(sampleRate, frequency, q);
    
    // Apply gain to the coefficients
    for (int i = 0; i < 6; ++i)
    {
        coefficients.coefficients[i] *= gain;
    }
    
    filter->setCoefficients(coefficients);
}