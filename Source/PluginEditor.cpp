/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ISODRONEAudioProcessorEditor::ISODRONEAudioProcessorEditor (ISODRONEAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), adsr(audioProcessor.apvts)
{
    setSize (600, 500);  // Made larger
    
    // Oscillator attachment
    oscSelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "OSC", oscSelector);
    
    // Glottal parameter attachments
    openQuotientAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "OPENQUOT", openQuotientSlider);
    asymmetryAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "ASYMMETRY", asymmetrySlider);
    breathinessAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "BREATHINESS", breathinessSlider);
    tensenessAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "TENSENESS", tensenessSlider);
    
    
    // Setup glottal parameter sliders
    setGlottalParams(openQuotientSlider);
    setGlottalParams(asymmetrySlider);
    setGlottalParams(breathinessSlider);
    setGlottalParams(tensenessSlider);
    
    // Setup oscillator selector
    oscSelector.addItem("Saw", 1);
    oscSelector.addItem("Glottal", 2);
    oscSelector.setSelectedId(2);  // Set default to Glottal
    addAndMakeVisible(oscSelector);
    
    addAndMakeVisible (adsr);

    // Setup glottal parameter labels
    openQuotientLabel.setText("Open Quotient", juce::dontSendNotification);
    openQuotientLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(openQuotientLabel);
    
    asymmetryLabel.setText("Asymmetry", juce::dontSendNotification);
    asymmetryLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(asymmetryLabel);
    
    breathinessLabel.setText("Breathiness", juce::dontSendNotification);
    breathinessLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(breathinessLabel);
    
    tensenessLabel.setText("Tenseness", juce::dontSendNotification);
    tensenessLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(tensenessLabel);
    
    // Oscillator label
    oscLabel.setText("Oscillator", juce::dontSendNotification);
    oscLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(oscLabel);
}

ISODRONEAudioProcessorEditor::~ISODRONEAudioProcessorEditor()
{
}

//==============================================================================
void ISODRONEAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void ISODRONEAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(20, 20);
    const auto padding = 15;
    
    // Oscillator selector at top
    juce::Rectangle<int> oscArea = bounds.removeFromTop(50);
    oscLabel.setBounds(oscArea.getX(), oscArea.getY(), 80, 20);
    oscSelector.setBounds(oscArea.getX() + 85, oscArea.getY(), 150, 30);

    // Add some space
    bounds.removeFromTop(20);
    
    // ADSR section - give it specific bounds instead of the entire area
    juce::Rectangle<int> adsrArea = bounds.removeFromTop(200);
    adsr.setBounds(adsrArea);

    // Add some space
    bounds.removeFromTop(20);

    // Glottal parameter sliders with labels
    const auto glottalSliderHeight = 40;
    const auto glottalLabelHeight = 20;
    const auto glottalSpacing = 10;
    const auto totalGlottalHeight = glottalSliderHeight + glottalLabelHeight + glottalSpacing;
    
    // Open Quotient
    openQuotientLabel.setBounds(bounds.getX(), bounds.getY(), bounds.getWidth(), glottalLabelHeight);
    openQuotientSlider.setBounds(bounds.getX(), bounds.getY() + glottalLabelHeight, bounds.getWidth(), glottalSliderHeight);
    
    // Asymmetry
    asymmetryLabel.setBounds(bounds.getX(), bounds.getY() + totalGlottalHeight, bounds.getWidth(), glottalLabelHeight);
    asymmetrySlider.setBounds(bounds.getX(), bounds.getY() + totalGlottalHeight + glottalLabelHeight, bounds.getWidth(), glottalSliderHeight);
    
    // Breathiness
    breathinessLabel.setBounds(bounds.getX(), bounds.getY() + 2 * totalGlottalHeight, bounds.getWidth(), glottalLabelHeight);
    breathinessSlider.setBounds(bounds.getX(), bounds.getY() + 2 * totalGlottalHeight + glottalLabelHeight, bounds.getWidth(), glottalSliderHeight);
    
    // Tenseness
    tensenessLabel.setBounds(bounds.getX(), bounds.getY() + 3 * totalGlottalHeight, bounds.getWidth(), glottalLabelHeight);
    tensenessSlider.setBounds(bounds.getX(), bounds.getY() + 3 * totalGlottalHeight + glottalLabelHeight, bounds.getWidth(), glottalSliderHeight);
}

void ISODRONEAudioProcessorEditor::setGlottalParams (juce::Slider& slider)
{
    slider.setSliderStyle (juce::Slider::SliderStyle::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 50, 25);
    addAndMakeVisible (slider);
}