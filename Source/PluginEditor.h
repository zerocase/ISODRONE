/*
  ==============================================================================

    PluginEditor.h - Back to original structure with just oscillator consolidation

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/ADSRComponent.h"
#include "GUI/OscComponent.h"

//==============================================================================
/**
*/
class ISODRONEAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ISODRONEAudioProcessorEditor(ISODRONEAudioProcessor&);
    ~ISODRONEAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // Type alias for cleaner code
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;

    void setGlottalParams(juce::Slider& slider);

    ISODRONEAudioProcessor& audioProcessor;
    
    // Components - OscComponent now just handles wave type selection
    OscComponent osc;
    ADSRComponent adsr;
    
    // Glottal parameter sliders - back in main editor for now
    juce::Slider openQuotientSlider;
    juce::Slider asymmetrySlider;
    juce::Slider breathinessSlider;
    juce::Slider tensenessSlider;

    // Glottal parameter labels
    juce::Label openQuotientLabel;
    juce::Label asymmetryLabel;
    juce::Label breathinessLabel;
    juce::Label tensenessLabel;

    juce::TextButton loadScalaButton;
    juce::TextButton loadKbmButton;
    juce::Label scalaStatusLabel;

    // Glottal parameter attachments
    std::unique_ptr<SliderAttachment> openQuotientAttachment;
    std::unique_ptr<SliderAttachment> asymmetryAttachment;
    std::unique_ptr<SliderAttachment> breathinessAttachment;
    std::unique_ptr<SliderAttachment> tensenessAttachment;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ISODRONEAudioProcessorEditor)
};