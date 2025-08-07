/*
  ==============================================================================

    IsoSound.h
    Created: 7 Aug 2025 7:50:55pm
    Author:  zerocase

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


class IsoSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int midiNoteNumber) override { return true; }
    bool appliesToChannel (int midiChannel) override { return true; }
};