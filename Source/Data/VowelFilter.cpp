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
    , vowelMorphValue(1.0f)  // Start at E vowel
    , currentFundamental(220.0f)
    , referenceFundamental(220.0f)
    , formantShift(1.0f)
    , formantSpread(1.0f)
    , bandwidthScale(1.0f)
    , resonanceGain(1.0f)
    , harmonicAlignment(false)
{
}

VowelFilter::~VowelFilter()
{
}

void VowelFilter::prepareToPlay(double newSampleRate, int samplesPerBlock)
{
    sampleRate = newSampleRate;
    
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

void VowelFilter::process(juce::AudioBuffer<float>& buffer)
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
            
            // Apply overall gain scaling with resonance control
            filteredSample *= (0.7f * resonanceGain);
            
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

// Main controls
void VowelFilter::setVowelType(VowelType vowel)
{
    if (currentVowel != vowel)
    {
        currentVowel = vowel;
        vowelMorphValue = static_cast<float>(vowel);
        updateFilters();
    }
}

void VowelFilter::setVowelMorph(float morphValue)
{
    // Clamp to valid range
    morphValue = juce::jlimit(0.0f, static_cast<float>(NumVowels - 1), morphValue);
    
    if (std::abs(vowelMorphValue - morphValue) > 0.01f)
    {
        vowelMorphValue = morphValue;
        
        // Update currentVowel to nearest vowel for reference
        currentVowel = static_cast<VowelType>(juce::roundToInt(morphValue));
        
        updateFilters();
    }
}

void VowelFilter::setFundamentalFrequency(float frequency)
{
    if (std::abs(frequency - currentFundamental) > 1.0f && frequency > 50.0f && frequency < 2000.0f)
    {
        currentFundamental = frequency;
        updateFilters();
    }
}

// Advanced filter controls
void VowelFilter::setFormantShift(float shiftFactor)
{
    shiftFactor = juce::jlimit(0.5f, 2.0f, shiftFactor);
    if (std::abs(formantShift - shiftFactor) > 0.01f)
    {
        formantShift = shiftFactor;
        updateFilters();
    }
}

void VowelFilter::setFormantSpread(float spreadFactor)
{
    spreadFactor = juce::jlimit(0.5f, 2.0f, spreadFactor);
    if (std::abs(formantSpread - spreadFactor) > 0.01f)
    {
        formantSpread = spreadFactor;
        updateFilters();
    }
}

void VowelFilter::setBandwidthScale(float bandwidthFactor)
{
    bandwidthFactor = juce::jlimit(0.5f, 3.0f, bandwidthFactor);
    if (std::abs(bandwidthScale - bandwidthFactor) > 0.01f)
    {
        bandwidthScale = bandwidthFactor;
        updateFilters();
    }
}

void VowelFilter::setResonanceGain(float gainFactor)
{
    resonanceGain = juce::jlimit(0.1f, 2.0f, gainFactor);
    // No need to update filters, this is applied in real-time during processing
}

void VowelFilter::setHarmonicAlignment(bool enabled)
{
    if (harmonicAlignment != enabled)
    {
        harmonicAlignment = enabled;
        updateFilters();
    }
}

// Internal methods
float VowelFilter::findNearestHarmonic(float formantFreq, float fundamental)
{
    if (fundamental <= 0.0f) return formantFreq;
    
    int harmonicNumber = juce::roundToInt(formantFreq / fundamental);
    if (harmonicNumber < 1) harmonicNumber = 1;
    
    return fundamental * harmonicNumber;
}

float VowelFilter::applyFormantAdjustments(float baseFrequency, int formantIndex)
{
    float adjustedFreq = baseFrequency;
    
    // Apply pitch-based scaling
    if (currentFundamental > 0.0f)
    {
        float pitchRatio = currentFundamental / referenceFundamental;
        adjustedFreq *= std::pow(pitchRatio, 0.25f);
    }
    
    // Apply global formant shift
    adjustedFreq *= formantShift;
    
    // Apply formant spreading (spread higher formants more)
    if (formantIndex > 0)
    {
        float spreadAmount = 1.0f + (formantSpread - 1.0f) * (formantIndex / 2.0f);
        adjustedFreq *= spreadAmount;
    }
    
    // Apply harmonic alignment if enabled
    if (harmonicAlignment && currentFundamental > 0.0f)
    {
        adjustedFreq = findNearestHarmonic(adjustedFreq, currentFundamental);
    }
    
    // Ensure reasonable limits
    adjustedFreq = juce::jlimit(baseFrequency * 0.5f, baseFrequency * 2.5f, adjustedFreq);
    
    return adjustedFreq;
}

// NEW: Interpolate between vowel formants
VowelFilter::FormantData VowelFilter::interpolateFormants(float morphValue)
{
    // Get the two vowels to interpolate between
    int vowel1 = static_cast<int>(std::floor(morphValue));
    int vowel2 = static_cast<int>(std::ceil(morphValue));
    
    // Clamp to valid range
    vowel1 = juce::jlimit(0, NumVowels - 1, vowel1);
    vowel2 = juce::jlimit(0, NumVowels - 1, vowel2);
    
    // Calculate interpolation factor (0.0 = vowel1, 1.0 = vowel2)
    float blend = morphValue - std::floor(morphValue);
    
    // Get formant data for both vowels
    const FormantData& formantA = vowelFormants[vowel1];
    const FormantData& formantB = vowelFormants[vowel2];
    
    // Interpolate all formant parameters
    FormantData result;
    result.f1 = formantA.f1 + blend * (formantB.f1 - formantA.f1);
    result.f2 = formantA.f2 + blend * (formantB.f2 - formantA.f2);
    result.f3 = formantA.f3 + blend * (formantB.f3 - formantA.f3);
    
    result.bw1 = formantA.bw1 + blend * (formantB.bw1 - formantA.bw1);
    result.bw2 = formantA.bw2 + blend * (formantB.bw2 - formantA.bw2);
    result.bw3 = formantA.bw3 + blend * (formantB.bw3 - formantA.bw3);
    
    result.gain1 = formantA.gain1 + blend * (formantB.gain1 - formantA.gain1);
    result.gain2 = formantA.gain2 + blend * (formantB.gain2 - formantA.gain2);
    result.gain3 = formantA.gain3 + blend * (formantB.gain3 - formantA.gain3);
    
    return result;
}

void VowelFilter::updateFilters()
{
    // Get interpolated formant data based on morphValue
    const FormantData formant = interpolateFormants(vowelMorphValue);
    
    // Update each formant filter for all channels
    for (int channel = 0; channel < static_cast<int>(formant1Filters.size()); ++channel)
    {
        if (formant1Filters[channel])
        {
            float freq = applyFormantAdjustments(formant.f1, 0);
            float bw = formant.bw1 * bandwidthScale;
            createBandpassFilter(formant1Filters[channel].get(), freq, bw, formant.gain1);
        }
            
        if (formant2Filters[channel])
        {
            float freq = applyFormantAdjustments(formant.f2, 1);
            float bw = formant.bw2 * bandwidthScale;
            createBandpassFilter(formant2Filters[channel].get(), freq, bw, formant.gain2);
        }
            
        if (formant3Filters[channel])
        {
            float freq = applyFormantAdjustments(formant.f3, 2);
            float bw = formant.bw3 * bandwidthScale;
            createBandpassFilter(formant3Filters[channel].get(), freq, bw, formant.gain3);
        }
    }
}

void VowelFilter::createBandpassFilter(juce::IIRFilter* filter, float frequency, float bandwidth, float gain)
{
    // Ensure frequency is within valid range
    frequency = juce::jlimit(50.0f, static_cast<float>(sampleRate * 0.4), frequency);
    bandwidth = juce::jlimit(20.0f, frequency * 0.5f, bandwidth);
    
    // Convert bandwidth to Q factor: Q = frequency / bandwidth
    float q = frequency / bandwidth;
    q = juce::jlimit(0.7f, 8.0f, q);
    
    // Create bandpass filter coefficients
    auto coefficients = juce::IIRCoefficients::makeBandPass(sampleRate, frequency, q);
    
    // Apply gain scaling
    float safeGain = juce::jlimit(0.1f, 2.0f, gain);
    
    // Scale coefficients properly - only scale the numerator (b coefficients)
    coefficients.coefficients[0] *= safeGain; // b0
    coefficients.coefficients[1] *= safeGain; // b1  
    coefficients.coefficients[2] *= safeGain; // b2
    
    filter->setCoefficients(coefficients);
}