/*
  ==============================================================================

    OscComponent.cpp - Updated for new styling

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
    oscWaveSelector.setSelectedId(2); // Default to Glottal (index 2)
    
    // Modern styling for ComboBox
    oscWaveSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a2a2a));
    oscWaveSelector.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    oscWaveSelector.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff4a9eff));
    oscWaveSelector.setColour(juce::ComboBox::arrowColourId, juce::Colour(0xff4a9eff));
    oscWaveSelector.setColour(juce::ComboBox::buttonColourId, juce::Colour(0xff333333));
    
    // Popup styling
    oscWaveSelector.setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff2a2a2a));
    oscWaveSelector.setColour(juce::PopupMenu::textColourId, juce::Colours::white);
    oscWaveSelector.setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff4a9eff));
    
    addAndMakeVisible(oscWaveSelector);

    // Label setup
    oscWaveSelectorLabel.setText("Oscillator Type", juce::dontSendNotification);
    oscWaveSelectorLabel.setJustificationType(juce::Justification::centred);
    oscWaveSelectorLabel.setColour(juce::Label::textColourId, juce::Colour(0xff4a9eff));
    oscWaveSelectorLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    addAndMakeVisible(oscWaveSelectorLabel);

    // Create attachment
    oscWaveSelectorAttachment = std::make_unique<ComboBoxAttachment>(apvts, waveSelectorId, oscWaveSelector);
}

OscComponent::~OscComponent()
{
}

void OscComponent::paint(juce::Graphics& g)
{
    // Completely transparent - let the main UI handle all drawing
    g.fillAll(juce::Colour(0x00000000));
}

void OscComponent::resized()
{
    auto area = getLocalBounds();
    
    // Section label at top with proper spacing
    auto labelArea = area.removeFromTop(30);
    oscWaveSelectorLabel.setBounds(labelArea);
    
    // Small gap
    area.removeFromTop(15);
    
    // ComboBox - make it smaller and centered
    auto comboHeight = 30;
    auto comboArea = area.removeFromTop(comboHeight);
    auto comboWidth = 50; // Fixed smaller width instead of responsive
    auto comboBounds = comboArea.withSizeKeepingCentre(comboWidth, comboHeight);
    oscWaveSelector.setBounds(comboBounds);
}