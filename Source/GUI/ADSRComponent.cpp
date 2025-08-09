/*
 ==============================================================================
 ADSRComponent.cpp
 Created: 9 Aug 2025 1:44:40pm
 Author: zerocase
 ==============================================================================
*/
#include <JuceHeader.h>
#include "ADSRComponent.h"

//==============================================================================
ADSRComponent::ADSRComponent(juce::AudioProcessorValueTreeState& apvts)
{
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    // ADSR Attachments
    attackAttachment = std::make_unique<SliderAttachment>(apvts, "ATTACK", attackSlider);
    decayAttachment = std::make_unique<SliderAttachment>(apvts, "DECAY", decaySlider);
    sustainAttachment = std::make_unique<SliderAttachment>(apvts, "SUSTAIN", sustainSlider);
    releaseAttachment = std::make_unique<SliderAttachment>(apvts, "RELEASE", releaseSlider);
    
    // Setup ADSR sliders
    setADSRParams(attackSlider);
    setADSRParams(decaySlider);
    setADSRParams(sustainSlider);
    setADSRParams(releaseSlider);
    
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
}

ADSRComponent::~ADSRComponent()
{
}

void ADSRComponent::paint (juce::Graphics& g)
{
    // Optional: Add a subtle background or border to distinguish the ADSR section
    g.fillAll(juce::Colours::transparentBlack);
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds(), 1);
}

void ADSRComponent::resized()
{
    auto bounds = getLocalBounds().reduced(10, 10);
    const auto padding = 15;
    
    // Calculate slider dimensions
    const auto sliderWidth = (bounds.getWidth() - 3 * padding) / 4;
    const auto labelHeight = 20;
    const auto sliderHeight = bounds.getHeight() - labelHeight - 10; // Leave room for labels and spacing
    
    // Position labels at the top
    attackLabel.setBounds(bounds.getX(), bounds.getY(), sliderWidth, labelHeight);
    decayLabel.setBounds(attackLabel.getRight() + padding, bounds.getY(), sliderWidth, labelHeight);
    sustainLabel.setBounds(decayLabel.getRight() + padding, bounds.getY(), sliderWidth, labelHeight);
    releaseLabel.setBounds(sustainLabel.getRight() + padding, bounds.getY(), sliderWidth, labelHeight);
    
    // Position sliders below labels
    const auto sliderY = bounds.getY() + labelHeight + 5; // 5px spacing between labels and sliders
    attackSlider.setBounds(bounds.getX(), sliderY, sliderWidth, sliderHeight);
    decaySlider.setBounds(attackSlider.getRight() + padding, sliderY, sliderWidth, sliderHeight);
    sustainSlider.setBounds(decaySlider.getRight() + padding, sliderY, sliderWidth, sliderHeight);
    releaseSlider.setBounds(sustainSlider.getRight() + padding, sliderY, sliderWidth, sliderHeight);
}

void ADSRComponent::setADSRParams (juce::Slider& slider)
{
    slider.setSliderStyle (juce::Slider::SliderStyle::LinearVertical);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 50, 25);
    addAndMakeVisible (slider);
}