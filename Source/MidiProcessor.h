/*
 ==============================================================================
 MidiProcessor.h - CORRECTED
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
    void setApvts(juce::AudioProcessorValueTreeState* apvtsPtr) { apvts = apvtsPtr; }
    
    // Scala file management
    void loadScalaFile();
    void loadKbmFile();
    double midiNoteToFrequency(int midiNote);
    
    // CC values from encoders - GLOTTAL PAGE (CC 14-17)
    std::atomic<float> openQuotient{0.6f};
    std::atomic<float> asymmetry{0.7f};
    std::atomic<float> breathiness{0.1f};
    std::atomic<float> tenseness{0.8f};
    
    // CC values from encoders - VOWEL PAGE (CC 18-21)
    std::atomic<float> formantShift{1.0f};
    std::atomic<float> formantSpread{1.0f};
    std::atomic<float> bandwidthScale{1.0f};
    std::atomic<float> resonanceGain{1.0f};
    
    // Gesture control - VOWEL MORPH (CC 11)
    std::atomic<float> vowelMorph{1.0f};  // 0.0-4.0 (A-E-I-O-U)
    
    // Gesture note tracking
    std::atomic<bool> gestureNoteActive{false};
    std::atomic<int> currentGestureNote{60};  // Default to C4
    
    // Current page (CC 119)
    std::atomic<int> currentPage{0};

private:
    juce::AudioProcessorValueTreeState* apvts = nullptr;
    
    scala::scale currentScale;
    scala::kbm currentKeyboardMapping;
    bool scalaFileLoaded = false;
    bool kbmFileLoaded = false;
    
    int frequencyToClosestMidiNote(double frequency);
    int calculatePitchBendForFrequency(int midiNote, double targetFrequency);
    
    // NEW: Separate CC handling
    void handleControlChange(const juce::MidiMessage& message);
    
    // Helper to map CC value (0-127) to parameter range
    float ccToRange(int ccValue, float min, float max)
    {
        return min + (ccValue / 127.0f) * (max - min);
    }
    
    // Keep FileChooser objects alive during async operations
    std::unique_ptr<juce::FileChooser> scalaFileChooser;
    std::unique_ptr<juce::FileChooser> kbmFileChooser;
};