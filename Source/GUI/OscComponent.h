/*
  ==============================================================================

    OscComponent.h - Updated for new styling

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class OscComponent : public juce::Component
{
public:
    OscComponent(juce::AudioProcessorValueTreeState& apvts, juce::String waveSelectorId);
    ~OscComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // Type alias for cleaner code
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    
    // Wave selector - core oscillator type selection
    juce::ComboBox oscWaveSelector;
    juce::Label oscWaveSelectorLabel;
    std::unique_ptr<ComboBoxAttachment> oscWaveSelectorAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscComponent)
};