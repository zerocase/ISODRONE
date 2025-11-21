/*
  ==============================================================================

    ADSRComponent.cpp - Updated for new styling

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
    setupLabel(attackLabel, "Attack");
    setupLabel(decayLabel, "Decay");
    setupLabel(sustainLabel, "Sustain");
    setupLabel(releaseLabel, "Release");
    
    // Setup section label
    adsrSectionLabel.setText("ENVELOPE", juce::dontSendNotification);
    adsrSectionLabel.setJustificationType(juce::Justification::centred);
    adsrSectionLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    adsrSectionLabel.setColour(juce::Label::textColourId, juce::Colour(0xff4a9eff));
    addAndMakeVisible(adsrSectionLabel);
}

ADSRComponent::~ADSRComponent()
{
}

void ADSRComponent::paint(juce::Graphics& g)
{
    // Completely transparent - let the main UI handle all drawing
    g.fillAll(juce::Colour(0x00000000));
}

void ADSRComponent::resized()
{
    auto bounds = getLocalBounds();
    
    // Section label at top
    auto labelArea = bounds.removeFromTop(30);
    adsrSectionLabel.setBounds(labelArea);
    
    // Small gap
    bounds.removeFromTop(15);
    
    const auto padding = 8;
    
    // Calculate slider dimensions - use remaining space efficiently
    const auto sliderWidth = (bounds.getWidth() - 3 * padding) / 4;
    const auto labelHeight = 20;
    const auto sliderHeight = bounds.getHeight() - labelHeight - 15;

    // Position labels at the top
    attackLabel.setBounds(bounds.getX(), bounds.getY(), sliderWidth, labelHeight);
    decayLabel.setBounds(attackLabel.getRight() + padding, bounds.getY(), sliderWidth, labelHeight);
    sustainLabel.setBounds(decayLabel.getRight() + padding, bounds.getY(), sliderWidth, labelHeight);
    releaseLabel.setBounds(sustainLabel.getRight() + padding, bounds.getY(), sliderWidth, labelHeight);

    // Position sliders below labels
    const auto sliderY = bounds.getY() + labelHeight + 10;
    attackSlider.setBounds(bounds.getX(), sliderY, sliderWidth, sliderHeight);
    decaySlider.setBounds(attackSlider.getRight() + padding, sliderY, sliderWidth, sliderHeight);
    sustainSlider.setBounds(decaySlider.getRight() + padding, sliderY, sliderWidth, sliderHeight);
    releaseSlider.setBounds(sustainSlider.getRight() + padding, sliderY, sliderWidth, sliderHeight);
}

void ADSRComponent::setADSRParams(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    
    // Modern slider styling
    slider.setColour(juce::Slider::trackColourId, juce::Colour(0xff333333));
    slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xff4a9eff));
    slider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1a1a1a));
    slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x00000000));
    slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
    
    addAndMakeVisible(slider);
}

void ADSRComponent::setupLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(11.0f));
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(label);
}