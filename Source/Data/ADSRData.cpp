/*
  ==============================================================================

    ADSRData.cpp
    Created: 9 Aug 2025 1:45:20pm
    Author:  zerocase

  ==============================================================================
*/

#include "ADSRData.h"


void ADSRData::updateADSR(const float attack, const float decay, const float sustain, const float release)
{
    adsrParams.attack = attack;
    adsrParams.decay = decay;
    adsrParams.sustain = sustain;
    adsrParams.release = release;

    setParameters (adsrParams);
};