/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ISODRONEAudioProcessorEditor::ISODRONEAudioProcessorEditor (ISODRONEAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (600, 500);  // Made larger
    
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    // ADSR Attachments
    attackAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "ATTACK", attackSlider);
    decayAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DECAY", decaySlider);
    sustainAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "SUSTAIN", sustainSlider);
    releaseAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "RELEASE", releaseSlider);
    
    // Oscillator attachment
    oscSelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "OSC", oscSelector);
    
    // Glottal parameter attachments
    openQuotientAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "OPENQUOT", openQuotientSlider);
    asymmetryAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "ASYMMETRY", asymmetrySlider);
    breathinessAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "BREATHINESS", breathinessSlider);
    tensenessAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "TENSENESS", tensenessSlider);
    
    // Setup ADSR sliders
    setADSRParams(attackSlider);
    setADSRParams(decaySlider);
    setADSRParams(sustainSlider);
    setADSRParams(releaseSlider);
    
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
    
    // Setup ADSR labels
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(attackLabel);
    
    decayLabel.setText("Decay", juce::dontSendNotification);
    decayLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(decayLabel);
    
    sustainLabel.setText("Sustain", juce::dontSendNotification);
    sustainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(sustainLabel);
    
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(releaseLabel);
    
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
    auto bounds = getLocalBounds().reduced(20, 20);  // More padding - removed const
    const auto padding = 15;
    
    // Oscillator selector at top
    juce::Rectangle<int> oscArea = bounds.removeFromTop(50);
    oscLabel.setBounds(oscArea.getX(), oscArea.getY(), 80, 20);
    oscSelector.setBounds(oscArea.getX() + 85, oscArea.getY(), 150, 30);
    
    // ADSR section
    juce::Rectangle<int> adsrArea = bounds.removeFromTop(200);
    const auto sliderWidth = (adsrArea.getWidth() - 3 * padding) / 4;
    const auto sliderHeight = adsrArea.getHeight() - 40; // Leave room for labels
    
    // ADSR labels (above sliders)
    attackLabel.setBounds(adsrArea.getX(), adsrArea.getY(), sliderWidth, 20);
    decayLabel.setBounds(attackLabel.getRight() + padding, adsrArea.getY(), sliderWidth, 20);
    sustainLabel.setBounds(decayLabel.getRight() + padding, adsrArea.getY(), sliderWidth, 20);
    releaseLabel.setBounds(sustainLabel.getRight() + padding, adsrArea.getY(), sliderWidth, 20);
    
    // ADSR sliders
    attackSlider.setBounds(adsrArea.getX(), adsrArea.getY() + 25, sliderWidth, sliderHeight);
    decaySlider.setBounds(attackSlider.getRight() + padding, adsrArea.getY() + 25, sliderWidth, sliderHeight);
    sustainSlider.setBounds(decaySlider.getRight() + padding, adsrArea.getY() + 25, sliderWidth, sliderHeight);
    releaseSlider.setBounds(sustainSlider.getRight() + padding, adsrArea.getY() + 25, sliderWidth, sliderHeight);
    
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

void ISODRONEAudioProcessorEditor::setADSRParams (juce::Slider& slider)
{
    slider.setSliderStyle (juce::Slider::SliderStyle::LinearVertical);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 50, 25);
    addAndMakeVisible (slider);
}

void ISODRONEAudioProcessorEditor::setGlottalParams (juce::Slider& slider)
{
    slider.setSliderStyle (juce::Slider::SliderStyle::LinearHorizontal);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 50, 25);
    addAndMakeVisible (slider);
}