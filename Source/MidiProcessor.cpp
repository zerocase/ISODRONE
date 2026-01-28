/*
 ==============================================================================
 MidiProcessor.cpp - CORRECTED
 Single-pass MIDI processing
 ==============================================================================
*/
#include "MidiProcessor.h"

void MidiProcessor::process(juce::MidiBuffer& midiMessages)
{
    // If Scala file is loaded, we need to process notes differently
    if (scalaFileLoaded)
    {
        juce::MidiBuffer processedMessages;
        
        for (const juce::MidiMessageMetadata metadata : midiMessages)
        {
            auto message = metadata.getMessage();
            
            // Handle CC messages
            if (message.isController())
            {
                handleControlChange(message);
                // Pass through CC messages unchanged
                processedMessages.addEvent(message, metadata.samplePosition);
            }
            // Handle Note On/Off with Scala processing
            else if (message.isNoteOn() || message.isNoteOff())
            {
                // Track gesture notes
                if (message.isNoteOn())
                {
                    gestureNoteActive = true;
                    currentGestureNote = message.getNoteNumber();
                    DBG("Gesture Note ON: " + juce::String(currentGestureNote));
                }
                else if (message.isNoteOff())
                {
                    gestureNoteActive = false;
                    DBG("Gesture Note OFF: " + juce::String(message.getNoteNumber()));
                }
                
                // Scala processing
                int originalMidiNote = message.getNoteNumber();
                double targetFrequency = midiNoteToFrequency(originalMidiNote);
                int closestMidiNote = frequencyToClosestMidiNote(targetFrequency);
                int pitchBendValue = calculatePitchBendForFrequency(closestMidiNote, targetFrequency);
                
                // Add pitch bend if needed
                if (pitchBendValue != 8192)
                {
                    auto pitchBendMsg = juce::MidiMessage::pitchWheel(message.getChannel(), pitchBendValue);
                    pitchBendMsg.setTimeStamp(metadata.samplePosition);
                    processedMessages.addEvent(pitchBendMsg, metadata.samplePosition);
                }
                
                // Add modified note
                juce::MidiMessage modifiedMessage;
                if (message.isNoteOn())
                {
                    modifiedMessage = juce::MidiMessage::noteOn(message.getChannel(), 
                                                              closestMidiNote, 
                                                              message.getVelocity());
                }
                else
                {
                    modifiedMessage = juce::MidiMessage::noteOff(message.getChannel(), 
                                                               closestMidiNote, 
                                                               message.getVelocity());
                }
                modifiedMessage.setTimeStamp(metadata.samplePosition);
                processedMessages.addEvent(modifiedMessage, metadata.samplePosition);
            }
            else
            {
                // Pass through other messages
                processedMessages.addEvent(message, metadata.samplePosition);
            }
        }
        
        // Replace buffer
        midiMessages = processedMessages;
    }
    else
    {
        // No Scala file - just process CCs and notes normally
        for (const juce::MidiMessageMetadata metadata : midiMessages)
        {
            auto message = metadata.getMessage();
            
            if (message.isController())
            {
                handleControlChange(message);
            }
            else if (message.isNoteOn())
            {
                gestureNoteActive = true;
                currentGestureNote = message.getNoteNumber();
                DBG("Gesture Note ON: " + juce::String(currentGestureNote));
            }
            else if (message.isNoteOff())
            {
                gestureNoteActive = false;
                DBG("Gesture Note OFF: " + juce::String(message.getNoteNumber()));
            }
        }
        // Messages pass through unchanged
    }
}

// NEW: Separate CC handling function
void MidiProcessor::handleControlChange(const juce::MidiMessage& message)
{
    int cc = message.getControllerNumber();
    int val = message.getControllerValue();
    
    switch (cc)
    {
        // GLOTTAL PAGE (CC 14-17)
        case 14: 
            openQuotient = ccToRange(val, 0.3f, 0.7f);
            if (apvts) apvts->getParameter("OPENQUOT")->setValueNotifyingHost(val / 127.0f);
            DBG("CC14 OpenQuotient: " + juce::String(openQuotient.load()));
            break;
        case 15: 
            asymmetry = ccToRange(val, 0.1f, 2.0f);
            if (apvts) apvts->getParameter("ASYMMETRY")->setValueNotifyingHost(val / 127.0f);
            DBG("CC15 Asymmetry: " + juce::String(asymmetry.load()));
            break;
        case 16: 
            breathiness = ccToRange(val, 0.0f, 1.0f);
            if (apvts) apvts->getParameter("BREATHINESS")->setValueNotifyingHost(val / 127.0f);
            DBG("CC16 Breathiness: " + juce::String(breathiness.load()));
            break;
        case 17: 
            tenseness = ccToRange(val, 0.0f, 1.0f);
            if (apvts) apvts->getParameter("TENSENESS")->setValueNotifyingHost(val / 127.0f);
            DBG("CC17 Tenseness: " + juce::String(tenseness.load()));
            break;
        
        // VOWEL PAGE (CC 18-21)
        case 18: 
            formantShift = ccToRange(val, 0.5f, 2.0f);
            if (apvts) apvts->getParameter("FORMANTSHIFT")->setValueNotifyingHost(val / 127.0f);
            DBG("CC18 FormantShift: " + juce::String(formantShift.load()));
            break;
        case 19: 
            formantSpread = ccToRange(val, 0.5f, 2.0f);
            if (apvts) apvts->getParameter("FORMANTSPREAD")->setValueNotifyingHost(val / 127.0f);
            DBG("CC19 FormantSpread: " + juce::String(formantSpread.load()));
            break;
        case 20: 
            bandwidthScale = ccToRange(val, 0.5f, 3.0f);
            if (apvts) apvts->getParameter("BANDWIDTHSCALE")->setValueNotifyingHost(val / 127.0f);
            DBG("CC20 BandwidthScale: " + juce::String(bandwidthScale.load()));
            break;
        case 21: 
            resonanceGain = ccToRange(val, 0.1f, 2.0f);
            if (apvts) apvts->getParameter("RESONANCEGAIN")->setValueNotifyingHost(val / 127.0f);
            DBG("CC21 ResonanceGain: " + juce::String(resonanceGain.load()));
            break;
        
        // GESTURE CONTROL - Vowel Morph (CC 11 - Expression)
        case 11: {
            // Map CC value (0-127) to vowel morph range (0.0-4.0)
            float vowelMorphValue = ccToRange(val, 0.0f, 4.0f);
            vowelMorph = vowelMorphValue;
            
            if (apvts) {
                auto* param = apvts->getParameter("VOWELMORPH");
                if (param) {
                    // Convert vowelMorphValue to normalized 0-1 range for the parameter
                    float normalized = vowelMorphValue / 4.0f;
                    param->setValueNotifyingHost(normalized);
                }
            }
            
            // Debug with vowel name
            const char* vowels[] = {"A", "E", "I", "O", "U"};
            int vowelIndex = static_cast<int>(vowelMorphValue);
            vowelIndex = juce::jlimit(0, 4, vowelIndex);
            DBG("CC11 Gesture -> VowelMorph: " + juce::String(vowelMorphValue, 2) + 
                " (" + juce::String(vowels[vowelIndex]) + ")");
            break;
        }
        
        // Page indicator (CC 119)
        case 119: 
            currentPage = val; 
            DBG("Page changed to: " + juce::String(val == 0 ? "GLOTTAL" : "VOWEL"));
            break;
    }
}

int MidiProcessor::frequencyToClosestMidiNote(double frequency)
{
    return juce::roundToInt(69.0 + 12.0 * std::log2(frequency / 440.0));
}

int MidiProcessor::calculatePitchBendForFrequency(int midiNote, double targetFrequency)
{
    double midiNoteFreq = juce::MidiMessage::getMidiNoteInHertz(midiNote);
    double cents = 1200.0 * std::log2(targetFrequency / midiNoteFreq);
    
    double bendRange = 200.0;
    int pitchBendValue = 8192 + juce::roundToInt((cents / bendRange) * 8191.0);
    
    return juce::jlimit(0, 16383, pitchBendValue);
}

void MidiProcessor::loadScalaFile()
{
    DBG("loadScalaFile() called - creating file chooser");
    
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    
    scalaFileChooser = std::make_unique<juce::FileChooser>("Select a Scala file", juce::File{}, "*.scl");
    
    scalaFileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        DBG("File chooser callback triggered");
        auto file = fc.getResult();
        
        if (file.existsAsFile())
        {
            DBG("Selected file: " + file.getFullPathName());
            std::ifstream scalaFile(file.getFullPathName().toStdString());
            
            if (scalaFile.is_open())
            {
                DBG("File opened successfully, parsing...");
                try 
                {
                    currentScale = scala::read_scl(scalaFile);
                    scalaFileLoaded = true;
                    DBG("Scala file loaded successfully: " + file.getFileName());
                }
                catch (const std::exception& e)
                {
                    DBG("Error loading Scala file: " + juce::String(e.what()));
                    scalaFileLoaded = false;
                }
                scalaFile.close();
            }
            else
            {
                DBG("Failed to open file for reading");
            }
        }
        else
        {
            DBG("No file selected or file chooser was cancelled");
        }
    });
}

void MidiProcessor::loadKbmFile()
{
    DBG("loadKbmFile() called - creating file chooser");
    
    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
    
    kbmFileChooser = std::make_unique<juce::FileChooser>("Select a KBM file", juce::File{}, "*.kbm");
    
    kbmFileChooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        DBG("KBM file chooser callback triggered");
        auto file = fc.getResult();
        
        if (file.existsAsFile())
        {
            DBG("Selected KBM file: " + file.getFullPathName());
            std::ifstream kbmFile(file.getFullPathName().toStdString());
            
            if (kbmFile.is_open())
            {
                DBG("KBM file opened successfully, parsing...");
                try 
                {
                    currentKeyboardMapping = scala::read_kbm(kbmFile);
                    kbmFileLoaded = true;
                    DBG("KBM file loaded successfully: " + file.getFileName());
                }
                catch (const std::exception& e)
                {
                    DBG("Error loading KBM file: " + juce::String(e.what()));
                    kbmFileLoaded = false;
                }
                kbmFile.close();
            }
            else
            {
                DBG("Failed to open KBM file for reading");
            }
        }
        else
        {
            DBG("No KBM file selected or file chooser was cancelled");
        }
    });
}

double MidiProcessor::midiNoteToFrequency(int midiNote)
{
    if (!scalaFileLoaded)
    {
        return 440.0 * std::pow(2.0, (midiNote - 69) / 12.0);
    }
    
    double referenceFreq = 440.0;
    int middleNote = 60;
    
    int noteOffset = midiNote - middleNote;
    int scaleLength = static_cast<int>(currentScale.get_scale_length()) - 1;
    
    if (scaleLength <= 0) return referenceFreq;
    
    int octaves = noteOffset / scaleLength;
    int scaleDegree = noteOffset % scaleLength;
    
    if (scaleDegree < 0)
    {
        scaleDegree += scaleLength;
        octaves--;
    }
    
    double ratio = currentScale.get_ratio(scaleDegree + 1);
    ratio *= std::pow(2.0, octaves);
    
    return referenceFreq * ratio;
}