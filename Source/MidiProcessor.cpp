/*
 ==============================================================================
 MidiProcessor.cpp
 Created: 19 Aug 2025 4:45:40pm
 Author: zerocase
 ==============================================================================
*/
#include "MidiProcessor.h"

void MidiProcessor::process(juce::MidiBuffer& midiMessages)
{
    if (!scalaFileLoaded)
    {
        // No Scala file loaded, just log and pass through unchanged
        for (const juce::MidiMessageMetadata metadata : midiMessages)
        {
            auto message = metadata.getMessage();
            DBG(message.getDescription());
        }
        return;
    }
    
    // Create a new buffer for modified messages
    juce::MidiBuffer processedMessages;
    
    for (const juce::MidiMessageMetadata metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        DBG(message.getDescription());
        
        if (message.isNoteOn() || message.isNoteOff())
        {
            int originalMidiNote = message.getNoteNumber();
            double targetFrequency = midiNoteToFrequency(originalMidiNote);
            
            // Find the closest MIDI note to this frequency
            int closestMidiNote = frequencyToClosestMidiNote(targetFrequency);
            
            // Calculate pitch bend needed to reach exact frequency
            int pitchBendValue = calculatePitchBendForFrequency(closestMidiNote, targetFrequency);
            
            DBG("MIDI note " + juce::String(originalMidiNote) + 
                " -> Target freq: " + juce::String(targetFrequency, 2) + "Hz" +
                " -> Closest note: " + juce::String(closestMidiNote) + 
                " -> Pitch bend: " + juce::String(pitchBendValue));
            
            // Add pitch bend message first (if needed)
            if (pitchBendValue != 8192) // 8192 is center/no bend
            {
                auto pitchBendMsg = juce::MidiMessage::pitchWheel(message.getChannel(), pitchBendValue);
                pitchBendMsg.setTimeStamp(metadata.samplePosition);
                processedMessages.addEvent(pitchBendMsg, metadata.samplePosition);
            }
            
            // Create modified note message with closest MIDI note
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
            // Pass through other MIDI messages unchanged
            processedMessages.addEvent(message, metadata.samplePosition);
        }
    }
    
    // Replace the original buffer with our processed one
    midiMessages = processedMessages;
}

int MidiProcessor::frequencyToClosestMidiNote(double frequency)
{
    // Convert frequency back to closest MIDI note number
    return juce::roundToInt(69.0 + 12.0 * std::log2(frequency / 440.0));
}

int MidiProcessor::calculatePitchBendForFrequency(int midiNote, double targetFrequency)
{
    double midiNoteFreq = juce::MidiMessage::getMidiNoteInHertz(midiNote);
    double cents = 1200.0 * std::log2(targetFrequency / midiNoteFreq);
    
    // Convert cents to pitch bend value (assuming ±2 semitone range = ±200 cents)
    // Pitch bend range: 0-16383, center is 8192
    double bendRange = 200.0; // ±200 cents (2 semitones)
    int pitchBendValue = 8192 + juce::roundToInt((cents / bendRange) * 8191.0);
    
    // Clamp to valid range
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
        // Fall back to 12-TET
        return 440.0 * std::pow(2.0, (midiNote - 69) / 12.0);
    }
    
    // Simple Scala conversion for now
    double referenceFreq = 440.0;
    int middleNote = 60; // C4
    
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
    
    double ratio = currentScale.get_ratio(scaleDegree + 1); // +1 because index 0 is 1/1
    ratio *= std::pow(2.0, octaves);
    
    return referenceFreq * ratio;
}