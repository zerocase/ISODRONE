/*
  ==============================================================================

    ADSRData.h
    Created: 9 Aug 2025 1:45:20pm
    Author:  zerocase

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


class ADSRData : public juce::ADSR
{
public:
    void updateADSR(const float attack, const float decay, const float sustain, const float release);
private:
    juce::ADSR::Parameters adsrParams;
};