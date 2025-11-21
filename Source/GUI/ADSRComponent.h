/*
  ==============================================================================

    ADSRComponent.h - Updated for new styling

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class ADSRComponent : public juce::Component
{
public:
    ADSRComponent(juce::AudioProcessorValueTreeState& apvts);
    ~ADSRComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;
    juce::Label adsrSectionLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> attackAttachment;
    std::unique_ptr<SliderAttachment> decayAttachment;
    std::unique_ptr<SliderAttachment> sustainAttachment;
    std::unique_ptr<SliderAttachment> releaseAttachment;

    void setADSRParams(juce::Slider& slider);
    void setupLabel(juce::Label& label, const juce::String& text);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ADSRComponent)
};