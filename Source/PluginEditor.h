/*
  ==============================================================================

    PluginEditor.h - Updated with FlexBox and Grid layout systems

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/ADSRComponent.h"
#include "GUI/OscComponent.h"

class ISODRONEAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ISODRONEAudioProcessorEditor(ISODRONEAudioProcessor&);
    ~ISODRONEAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // Main audio processor reference
    ISODRONEAudioProcessor& audioProcessor;

    // UI Components
    OscComponent osc;
    ADSRComponent adsr;

    // Glottal synthesis controls (knobs)
    juce::Slider openQuotientKnob, asymmetryKnob, breathinessKnob, tensenessKnob;
    juce::Label openQuotientLabel, asymmetryLabel, breathinessLabel, tensenessLabel;
    juce::Label glottalSectionLabel;

    // Vowel filter controls (knobs)
    juce::ComboBox vowelSelector;
    juce::Slider formantShiftKnob, formantSpreadKnob, bandwidthKnob, resonanceKnob;
    juce::Label vowelLabel, formantShiftLabel, formantSpreadLabel, bandwidthLabel, resonanceLabel;
    juce::ToggleButton harmonicAlignmentButton;
    juce::Label vowelSectionLabel;

    // Scala/microtonal controls
    juce::TextButton loadScalaButton, loadKbmButton;
    juce::Label scalaStatusLabel;
    juce::Label microtuningSectionLabel;

    // Parameter attachments
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    // Glottal parameter attachments
    std::unique_ptr<SliderAttachment> openQuotientAttachment;
    std::unique_ptr<SliderAttachment> asymmetryAttachment;
    std::unique_ptr<SliderAttachment> breathinessAttachment;
    std::unique_ptr<SliderAttachment> tensenessAttachment;

    // Vowel filter attachments
    std::unique_ptr<ComboBoxAttachment> vowelAttachment;
    std::unique_ptr<SliderAttachment> formantShiftAttachment;
    std::unique_ptr<SliderAttachment> formantSpreadAttachment;
    std::unique_ptr<SliderAttachment> bandwidthAttachment;
    std::unique_ptr<SliderAttachment> resonanceAttachment;
    std::unique_ptr<ButtonAttachment> harmonicAlignmentAttachment;

    // Layout helper methods
    void layoutMicrotuningSection(juce::Rectangle<int> area);
    void layoutOscillatorSection(juce::Rectangle<int> area);
    void layoutGlottalSection(juce::Rectangle<int> area);
    void layoutVowelSection(juce::Rectangle<int> area);

    // Component setup helper methods
    void setupKnob(juce::Slider& knob);
    void setupKnobValueDisplay(juce::Slider& knob, const juce::String& parameterID);
    void setupLabel(juce::Label& label, const juce::String& text, juce::Component& attachTo);
    void setupSectionLabel(juce::Label& label, const juce::String& text);

    // For storing section bounds for painting borders
    juce::Array<juce::Rectangle<int>> sectionBounds;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ISODRONEAudioProcessorEditor)
};