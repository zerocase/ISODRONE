/*
 ==============================================================================
 OscComponent.cpp
 Created: 9 Aug 2025 4:46:07pm
 Author: zerocase
 ==============================================================================
*/
#include <JuceHeader.h>
#include "OscComponent.h"

//==============================================================================
OscComponent::OscComponent(juce::AudioProcessorValueTreeState& apvts, juce::String waveSelectorId)
{
    // Wave selector setup
    juce::StringArray choices {"Sawtooth", "Glottal"};
    oscWaveSelector.addItemList(choices, 1);
    oscWaveSelector.setSelectedId(1);
    
    // Simple, clean styling
    oscWaveSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colours::white);
    oscWaveSelector.setColour(juce::ComboBox::textColourId, juce::Colours::black);
    oscWaveSelector.setColour(juce::ComboBox::outlineColourId, juce::Colours::grey);
    
    addAndMakeVisible(oscWaveSelector);
    
    // Label setup
    oscWaveSelectorLabel.setText("Oscillator Type", juce::dontSendNotification);
    oscWaveSelectorLabel.setJustificationType(juce::Justification::centredLeft);
    oscWaveSelectorLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(oscWaveSelectorLabel);
    
    // Create attachment
    oscWaveSelectorAttachment = std::make_unique<ComboBoxAttachment>(apvts, waveSelectorId, oscWaveSelector);
}

OscComponent::~OscComponent()
{
}

void OscComponent::paint(juce::Graphics& g)
{
    // Simple background - no weird styling
    g.fillAll(juce::Colours::darkgrey);
}

void OscComponent::resized()
{
    // Get the full component area
    auto area = getLocalBounds();
    
    // Clear separation - label at top, combo below with gap
    auto labelBounds = juce::Rectangle<int>(0, 0, getWidth(), 20);
    auto comboBounds = juce::Rectangle<int>(0, 25, getWidth(), 30);
    
    oscWaveSelectorLabel.setBounds(labelBounds);
    oscWaveSelector.setBounds(comboBounds);
    
    // Debug output
    DBG("OscComponent size: " << getWidth() << "x" << getHeight());
    DBG("Label bounds: " << labelBounds.toString());
    DBG("Combo bounds: " << comboBounds.toString());
}