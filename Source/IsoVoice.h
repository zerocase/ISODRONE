/*
  ==============================================================================

    IsoVoice.h
    Created: 7 Aug 2025 7:50:21pm
    Author:  zerocase

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "IsoSound.h"

class IsoVoice : public juce::SynthesiserVoice
{
  public:
    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound *sound, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void controllerMoved (int controllerNumber, int newControllerValue) override;
    void pitchWheelMoved (int newPitchWheelValue) override;
    void renderNextBlock (juce::AudioBuffer< float > &outputBuffer, int startSample, int numSamples) override;

  private:
};
