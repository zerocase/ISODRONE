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
    osc.setFrequency (juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber));
    adsr.noteOn();
}

void IsoVoice::stopNote (float velocity, bool allowTailOff)
{
    adsr.noteOff();
}

void IsoVoice::controllerMoved (int controllerNumber, int newControllerValue)
{

}

void IsoVoice::pitchWheelMoved (int newPitchWheelValue)
{

}

void IsoVoice::prepareToPlay (double sampleRate, int samplesPerBlock, int outputChannels)
{
    adsr.setSampleRate (sampleRate);
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = outputChannels;

    osc.prepare (spec);
    gain.prepare (spec);


    gain.setGainLinear (0.03f);

    isPrepared = true;
}

void IsoVoice::renderNextBlock (juce::AudioBuffer< float > &outputBuffer, int startSample, int numSamples)
{
    jassert (isPrepared);

    juce::dsp::AudioBlock<float> audioBlock { outputBuffer, (size_t)startSample };
    osc.process (juce::dsp::ProcessContextReplacing<float> (audioBlock));
    gain.process (juce::dsp::ProcessContextReplacing<float> (audioBlock));

    adsr.applyEnvelopeToBuffer (outputBuffer, startSample, numSamples);
}