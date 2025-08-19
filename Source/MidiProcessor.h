/*
 ==============================================================================
 MidiProcessor.h
 Created: 18 Aug 2025 10:30:47pm
 Author: zerocase
 ==============================================================================
*/
#pragma once
#include "JuceHeader.h"
#include "Data/ScalaFile.h"

class MidiProcessor
{
public:
    void process(juce::MidiBuffer& midiMessages);
    
    // Scala file management
    void loadScalaFile();
    void loadKbmFile();
    
private:
    scala::scale currentScale;
    scala::kbm currentKeyboardMapping;
    bool scalaFileLoaded = false;
    bool kbmFileLoaded = false;
    
    double midiNoteToFrequency(int midiNote);
    int frequencyToClosestMidiNote(double frequency);
    int calculatePitchBendForFrequency(int midiNote, double targetFrequency);
        
    // Keep FileChooser objects alive during async operations
    std::unique_ptr<juce::FileChooser> scalaFileChooser;
    std::unique_ptr<juce::FileChooser> kbmFileChooser;
};