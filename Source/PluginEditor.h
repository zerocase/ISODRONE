/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/ADSRComponent.h"
//==============================================================================
/**
*/
class ISODRONEAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ISODRONEAudioProcessorEditor (ISODRONEAudioProcessor&);
    ~ISODRONEAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Type alias for cleaner code
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    
    void setGlottalParams (juce::Slider& slider);
    void setupLabel (juce::Label& label, const juce::String& text);

    ISODRONEAudioProcessor& audioProcessor;
    ADSRComponent adsr;

    // Oscillator selector
    juce::ComboBox oscSelector;
    juce::Label oscLabel;

    // Glottal parameter sliders
    juce::Slider openQuotientSlider;
    juce::Slider asymmetrySlider;
    juce::Slider breathinessSlider;
    juce::Slider tensenessSlider;

    // Glottal parameter labels
    juce::Label openQuotientLabel;
    juce::Label asymmetryLabel;
    juce::Label breathinessLabel;
    juce::Label tensenessLabel;

    // Oscillator attachment
    std::unique_ptr<ComboBoxAttachment> oscSelAttachment;

    // Glottal parameter attachments
    std::unique_ptr<SliderAttachment> openQuotientAttachment;
    std::unique_ptr<SliderAttachment> asymmetryAttachment;
    std::unique_ptr<SliderAttachment> breathinessAttachment;
    std::unique_ptr<SliderAttachment> tensenessAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ISODRONEAudioProcessorEditor)
};