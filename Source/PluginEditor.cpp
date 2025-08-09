/*
  ==============================================================================

    PluginEditor.cpp - Back to original structure with consolidated oscillator

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ISODRONEAudioProcessorEditor::ISODRONEAudioProcessorEditor(ISODRONEAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), osc(audioProcessor.apvts, "OSC1WAVETYPE"), adsr(audioProcessor.apvts)
{
    setSize(600, 650);
    
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
    
    // Add components
    addAndMakeVisible(osc);
    addAndMakeVisible(adsr);

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
}

ISODRONEAudioProcessorEditor::~ISODRONEAudioProcessorEditor()
{
}

//==============================================================================
void ISODRONEAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void ISODRONEAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(20, 20);
    
    // Give oscillator component enough space and make sure it's properly sized
    auto oscArea = bounds.removeFromTop(80);  // Plenty of space
    osc.setBounds(oscArea);  // Use the full area, let the component handle internal layout
    
    DBG("Oscillator area: " << oscArea.toString());
    
    // Add gap
    bounds.removeFromTop(20);
    
    // Rest of layout...
    auto adsrArea = bounds.removeFromTop(200);
    adsr.setBounds(adsrArea);
    
    // Glottal parameters...
    bounds.removeFromTop(20);
    
    const auto sliderHeight = 60;  // More space for each slider+label combo
    
    // Open Quotient
    auto openQuotArea = bounds.removeFromTop(sliderHeight);
    auto openLabelArea = openQuotArea.removeFromTop(20);
    openQuotientLabel.setBounds(openLabelArea);
    openQuotientSlider.setBounds(openQuotArea);
    
    // Asymmetry  
    auto asymmetryArea = bounds.removeFromTop(sliderHeight);
    auto asymLabelArea = asymmetryArea.removeFromTop(20);
    asymmetryLabel.setBounds(asymLabelArea);
    asymmetrySlider.setBounds(asymmetryArea);
    
    // Breathiness
    auto breathArea = bounds.removeFromTop(sliderHeight);
    auto breathLabelArea = breathArea.removeFromTop(20);
    breathinessLabel.setBounds(breathLabelArea);
    breathinessSlider.setBounds(breathArea);
    
    // Tenseness
    auto tenseArea = bounds.removeFromTop(sliderHeight);
    auto tenseLabelArea = tenseArea.removeFromTop(20);
    tensenessLabel.setBounds(tenseLabelArea);
    tensenessSlider.setBounds(tenseArea);
}

void ISODRONEAudioProcessorEditor::setGlottalParams(juce::Slider& slider)
{
    slider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 50, 25);
    addAndMakeVisible(slider);
}