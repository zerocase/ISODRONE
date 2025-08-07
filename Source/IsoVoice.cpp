/*
  ==============================================================================

    IsoVoice.cpp
    Created: 7 Aug 2025 7:50:21pm
    Author:  zerocase

  ==============================================================================
*/

#include "IsoVoice.h"

bool IsoVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<juce::SynthesiserSound*>(sound) != nullptr;
}


void IsoVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound *sound, int currentPitchWheelPosition)
{

}

void IsoVoice::stopNote (float velocity, bool allowTailOff)
{

}

void IsoVoice::controllerMoved (int controllerNumber, int newControllerValue)
{

}

void IsoVoice::pitchWheelMoved (int newPitchWheelValue)
{

}

void IsoVoice::renderNextBlock (juce::AudioBuffer< float > &outputBuffer, int startSample, int numSamples)
{

}