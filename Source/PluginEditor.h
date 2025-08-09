/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

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
    void setADSRParams (juce::Slider& slider);
    void setGlottalParams (juce::Slider& slider);
    void setupLabel (juce::Label& label, const juce::String& text);

    // ADSR Sliders
    juce::Slider attackSlider;
    juce::Slider decaySlider;
    juce::Slider sustainSlider;
    juce::Slider releaseSlider;
    
    // ADSR Labels
    juce::Label attackLabel;
    juce::Label decayLabel;
    juce::Label sustainLabel;
    juce::Label releaseLabel;
    
    // Oscillator selector
    juce::ComboBox oscSelector;
    juce::Label oscLabel;
    
    // NEW: Glottal parameter sliders (these were missing!)
    juce::Slider openQuotientSlider;
    juce::Slider asymmetrySlider;
    juce::Slider breathinessSlider;
    juce::Slider tensenessSlider;
    
    // Glottal parameter labels
    juce::Label openQuotientLabel;
    juce::Label asymmetryLabel;
    juce::Label breathinessLabel;
    juce::Label tensenessLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    
    // ADSR Attachments
    std::unique_ptr<SliderAttachment> attackAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<SliderAttachment> sustainAttachment;
    std::unique_ptr<SliderAttachment> releaseAttachment;
    
    // Oscillator attachment
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> oscSelAttachment;
    
    // NEW: Glottal parameter attachments (these were missing!)
    std::unique_ptr<SliderAttachment> openQuotientAttachment;
    std::unique_ptr<SliderAttachment> asymmetryAttachment;
    std::unique_ptr<SliderAttachment> breathinessAttachment;
    std::unique_ptr<SliderAttachment> tensenessAttachment;

    ISODRONEAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ISODRONEAudioProcessorEditor)
};