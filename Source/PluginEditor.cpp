/*
  ==============================================================================

    PluginEditor.cpp - Updated with FlexBox and Grid layout systems

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ISODRONEAudioProcessorEditor::ISODRONEAudioProcessorEditor(ISODRONEAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), osc(audioProcessor.apvts, "OSC1WAVETYPE"), adsr(audioProcessor.apvts)
{
    setSize(450, 990); // Increased from 950 to 990 for more space
    
    // Create parameter attachments and setup custom value display
    openQuotientAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "OPENQUOT", openQuotientKnob);
    asymmetryAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "ASYMMETRY", asymmetryKnob);
    breathinessAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "BREATHINESS", breathinessKnob);
    tensenessAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "TENSENESS", tensenessKnob);
    
    vowelMorphAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "VOWELMORPH", vowelMorphSlider);
    formantShiftAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "FORMANTSHIFT", formantShiftKnob);
    formantSpreadAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "FORMANTSPREAD", formantSpreadKnob);
    bandwidthAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "BANDWIDTHSCALE", bandwidthKnob);
    resonanceAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "RESONANCEGAIN", resonanceKnob);
    harmonicAlignmentAttachment = std::make_unique<ButtonAttachment>(audioProcessor.apvts, "HARMONICALIGN", harmonicAlignmentButton);
    
    // Setup custom value displays AFTER attachments are created
    setupKnobValueDisplay(openQuotientKnob, "OPENQUOT");
    setupKnobValueDisplay(asymmetryKnob, "ASYMMETRY");
    setupKnobValueDisplay(breathinessKnob, "BREATHINESS");
    setupKnobValueDisplay(tensenessKnob, "TENSENESS");
    setupKnobValueDisplay(formantShiftKnob, "FORMANTSHIFT");
    setupKnobValueDisplay(formantSpreadKnob, "FORMANTSPREAD");
    setupKnobValueDisplay(bandwidthKnob, "BANDWIDTHSCALE");
    setupKnobValueDisplay(resonanceKnob, "RESONANCEGAIN");
    
    // Setup main components
    addAndMakeVisible(osc);
    addAndMakeVisible(adsr);

    // Setup section labels
    setupSectionLabel(glottalSectionLabel, "GLOTTAL SYNTHESIS");
    setupSectionLabel(vowelSectionLabel, "VOWEL FILTER");
    setupSectionLabel(microtuningSectionLabel, "MICROTUNING");

    // Setup glottal parameter knobs and labels
    openQuotientKnob.setName("OPENQUOT");
    asymmetryKnob.setName("ASYMMETRY");
    breathinessKnob.setName("BREATHINESS");
    tensenessKnob.setName("TENSENESS");
    
    setupKnob(openQuotientKnob);
    setupKnob(asymmetryKnob);
    setupKnob(breathinessKnob);
    setupKnob(tensenessKnob);
    
    setupLabel(openQuotientLabel, "Open\nQuotient", openQuotientKnob);
    setupLabel(asymmetryLabel, "Asymmetry", asymmetryKnob);
    setupLabel(breathinessLabel, "Breathiness", breathinessKnob);
    setupLabel(tensenessLabel, "Tenseness", tensenessKnob);

    // Setup vowel morph slider (horizontal, not rotary) - MUST BE BEFORE OTHER VOWEL KNOBS
    vowelMorphSlider.setName("VOWELMORPH");
    vowelMorphSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    vowelMorphSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    vowelMorphSlider.setColour(juce::Slider::trackColourId, juce::Colour(0xff4a9eff));
    vowelMorphSlider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff333333));
    vowelMorphSlider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    vowelMorphSlider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x00000000));
    vowelMorphSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
    setupKnobValueDisplay(vowelMorphSlider, "VOWELMORPH");
    addAndMakeVisible(vowelMorphSlider);
    
    setupLabel(vowelLabel, "Vowel", vowelMorphSlider);

    // Setup vowel filter knobs and labels
    formantShiftKnob.setName("FORMANTSHIFT");
    formantSpreadKnob.setName("FORMANTSPREAD");
    bandwidthKnob.setName("BANDWIDTHSCALE");
    resonanceKnob.setName("RESONANCEGAIN");
    
    setupKnob(formantShiftKnob);
    setupKnob(formantSpreadKnob);
    setupKnob(bandwidthKnob);
    setupKnob(resonanceKnob);
    
    setupLabel(formantShiftLabel, "Formant\nShift", formantShiftKnob);
    setupLabel(formantSpreadLabel, "Formant\nSpread", formantSpreadKnob);
    setupLabel(bandwidthLabel, "Bandwidth", bandwidthKnob);
    setupLabel(resonanceLabel, "Resonance", resonanceKnob);

    // Setup harmonic alignment button
    harmonicAlignmentButton.setButtonText("Harmonic\nAlignment");
    harmonicAlignmentButton.setClickingTogglesState(true);
    addAndMakeVisible(harmonicAlignmentButton);

    // Setup Scala file loading components
    loadScalaButton.setButtonText("Load .scl");
    loadScalaButton.onClick = [this] { 
        audioProcessor.midiProcessor.loadScalaFile(); 
        scalaStatusLabel.setText("Scala file loaded", juce::dontSendNotification);
    };
    addAndMakeVisible(loadScalaButton);

    loadKbmButton.setButtonText("Load .kbm"); 
    loadKbmButton.onClick = [this] { 
        audioProcessor.midiProcessor.loadKbmFile(); 
    };
    addAndMakeVisible(loadKbmButton);

    scalaStatusLabel.setText("12-TET (default)", juce::dontSendNotification);
    scalaStatusLabel.setJustificationType(juce::Justification::centred);
    scalaStatusLabel.setFont(juce::Font(12.0f));
    addAndMakeVisible(scalaStatusLabel);
}

ISODRONEAudioProcessorEditor::~ISODRONEAudioProcessorEditor()
{
}

//==============================================================================
void ISODRONEAudioProcessorEditor::paint(juce::Graphics& g)
{
    // Background gradient
    juce::ColourGradient gradient(juce::Colour(0xff1a1a1a), 0, 0,
                                  juce::Colour(0xff0f0f0f), 0, getHeight(),
                                  false);
    g.setGradientFill(gradient);
    g.fillAll();
    
    // Draw subtle section boundaries
    g.setColour(juce::Colour(0xff333333));
    
    // Draw borders around each section (will be calculated in resized())
    for (const auto& rect : sectionBounds)
    {
        g.drawRoundedRectangle(rect.toFloat(), 5.0f, 1.0f);
    }
}

void ISODRONEAudioProcessorEditor::resized()
{
    sectionBounds.clear(); // Clear previous bounds for painting
    
    auto bounds = getLocalBounds().reduced(15, 15); // More padding
    
    // 1. MICROTUNING SECTION (Top, compact)
    auto microtuningArea = bounds.removeFromTop(70); // Taller
    sectionBounds.add(microtuningArea);
    layoutMicrotuningSection(microtuningArea);
    
    bounds.removeFromTop(15); // Bigger gap
    
    // 2. OSCILLATOR SECTION (Compact)
    auto oscArea = bounds.removeFromTop(90); // Taller
    sectionBounds.add(oscArea);
    layoutOscillatorSection(oscArea);
    
    bounds.removeFromTop(15); // Bigger gap
    
    // 3. GLOTTAL SYNTHESIS SECTION (2x2 knobs)
    auto glottalArea = bounds.removeFromTop(220); // Taller
    sectionBounds.add(glottalArea);
    layoutGlottalSection(glottalArea);
    
    bounds.removeFromTop(15); // Bigger gap
    
    // 4. VOWEL FILTER SECTION (2x2 knobs + controls)
    auto vowelArea = bounds.removeFromTop(300); // Increased from 260 to 300
    sectionBounds.add(vowelArea);
    layoutVowelSection(vowelArea);
    
    bounds.removeFromTop(15); // Bigger gap
    
    // 5. ADSR SECTION (Bottom, remaining space)
    auto adsrArea = bounds;
    sectionBounds.add(adsrArea);
    adsr.setBounds(adsrArea);
}

void ISODRONEAudioProcessorEditor::layoutMicrotuningSection(juce::Rectangle<int> area)
{
    auto content = area.reduced(15, 12); // More padding
    
    // Compact microtuning layout
    juce::FlexBox buttonRow;
    buttonRow.flexDirection = juce::FlexBox::Direction::row;
    buttonRow.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    buttonRow.alignItems = juce::FlexBox::AlignItems::center;
    
    buttonRow.items.add(juce::FlexItem(loadScalaButton).withWidth(90).withHeight(30)); // Bigger buttons
    buttonRow.items.add(juce::FlexItem().withWidth(15)); // Bigger gap
    buttonRow.items.add(juce::FlexItem(loadKbmButton).withWidth(90).withHeight(30));
    buttonRow.items.add(juce::FlexItem().withWidth(15)); // Bigger gap
    buttonRow.items.add(juce::FlexItem(scalaStatusLabel).withFlex(1).withHeight(30));
    
    buttonRow.performLayout(content);
    
    // Hide the microtuning section label for compact design
    microtuningSectionLabel.setBounds(0, 0, 0, 0);
}

void ISODRONEAudioProcessorEditor::layoutOscillatorSection(juce::Rectangle<int> area)
{
    auto content = area.reduced(15, 12); // More padding
    
    // Simple oscillator section
    osc.setBounds(content);
}

void ISODRONEAudioProcessorEditor::layoutGlottalSection(juce::Rectangle<int> area)
{
    auto content = area.reduced(15, 15); // More padding
    
    // Section label
    auto labelArea = content.removeFromTop(25); // Taller label area
    glottalSectionLabel.setBounds(labelArea);
    
    content.removeFromTop(10); // More space after label
    
    // Use Grid for 2x2 knob layout
    juce::Grid glottalGrid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    glottalGrid.templateRows = { Track(Fr(1)), Track(Fr(1)) };
    glottalGrid.templateColumns = { Track(Fr(1)), Track(Fr(1)) };
    glottalGrid.rowGap = juce::Grid::Px(15); // More gap
    glottalGrid.columnGap = juce::Grid::Px(15); // More gap
    
    glottalGrid.items = { 
        juce::GridItem(openQuotientKnob),
        juce::GridItem(asymmetryKnob),
        juce::GridItem(breathinessKnob),
        juce::GridItem(tensenessKnob)
    };
    
    glottalGrid.performLayout(content);
}

void ISODRONEAudioProcessorEditor::layoutVowelSection(juce::Rectangle<int> area)
{
    auto content = area.reduced(15, 15); // More padding
    
    // Section label
    auto labelArea = content.removeFromTop(25); // Taller label area
    vowelSectionLabel.setBounds(labelArea);
    
    content.removeFromTop(10); // More space after label
    
    // Vowel morph slider (full width on top)
    auto vowelMorphArea = content.removeFromTop(30);
    vowelMorphSlider.setBounds(vowelMorphArea);
    
    content.removeFromTop(8); // Small gap
    
    // Harmonic alignment button (full width below slider)
    auto harmonicAlignArea = content.removeFromTop(30);
    harmonicAlignmentButton.setBounds(harmonicAlignArea);
    
    content.removeFromTop(15); // More space before knobs
    
    // Use Grid for 2x2 vowel knob layout
    juce::Grid vowelGrid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    vowelGrid.templateRows = { Track(Fr(1)), Track(Fr(1)) };
    vowelGrid.templateColumns = { Track(Fr(1)), Track(Fr(1)) };
    vowelGrid.rowGap = juce::Grid::Px(15); // More gap
    vowelGrid.columnGap = juce::Grid::Px(15); // More gap
    
    vowelGrid.items = { 
        juce::GridItem(formantShiftKnob),
        juce::GridItem(formantSpreadKnob),
        juce::GridItem(bandwidthKnob),
        juce::GridItem(resonanceKnob)
    };
    
    vowelGrid.performLayout(content);
    
}

void ISODRONEAudioProcessorEditor::setupKnob(juce::Slider& knob)
{
    knob.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
    knob.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff4a9eff));
    knob.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff333333));
    knob.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
    knob.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0x00000000));
    knob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0x00000000));
    
    addAndMakeVisible(knob);
}

void ISODRONEAudioProcessorEditor::setupKnobValueDisplay(juce::Slider& knob, const juce::String& parameterID)
{
    // Set up custom value display after attachment is created
    knob.textFromValueFunction = [parameterID](double value) -> juce::String 
    {
        if (parameterID == "OPENQUOT")
        {
            // Open quotient as percentage of cycle
            return juce::String(juce::roundToInt(value * 100)) + "%";
        }
        else if (parameterID == "ASYMMETRY")
        {
            // Asymmetry ratio
            return juce::String(value, 2);
        }
        else if (parameterID == "BREATHINESS")
        {
            // Breathiness as percentage
            return juce::String(juce::roundToInt(value * 100)) + "%";
        }
        else if (parameterID == "TENSENESS")
        {
            // Tenseness as percentage
            return juce::String(juce::roundToInt(value * 100)) + "%";
        }
        else if (parameterID == "FORMANTSHIFT")
        {
            // Formant shift in cents (assuming 0.5-2.0 range maps to ±1200 cents)
            double cents = (value - 1.0) * 1200.0;
            if (cents >= 0)
                return "+" + juce::String(juce::roundToInt(cents)) + "¢";
            else
                return juce::String(juce::roundToInt(cents)) + "¢";
        }
        else if (parameterID == "FORMANTSPREAD")
        {
            // Formant spread as percentage change
            double percent = (value - 1.0) * 100.0;
            if (percent >= 0)
                return "+" + juce::String(juce::roundToInt(percent)) + "%";
            else
                return juce::String(juce::roundToInt(percent)) + "%";
        }
        else if (parameterID == "BANDWIDTHSCALE")
        {
            // Bandwidth scaling factor
            if (value > 1.0)
                return juce::String(value, 1) + "x wide";
            else if (value < 1.0)
                return juce::String(value, 1) + "x narrow";
            else
                return "normal";
        }
        else if (parameterID == "RESONANCEGAIN")
        {
            // Resonance gain in dB
            double dB = 20.0 * std::log10(value);
            if (dB >= 0)
                return "+" + juce::String(dB, 1) + " dB";
            else
                return juce::String(dB, 1) + " dB";
        }
        else if (parameterID == "VOWELMORPH")
        {
            // Display vowel name with morphing indicator
            const char* vowels[] = {"A", "E", "I", "O", "U"};
            int baseVowel = static_cast<int>(std::floor(value));
            baseVowel = juce::jlimit(0, 4, baseVowel);
            
            double blend = value - std::floor(value);
            
            if (blend < 0.1)
                return juce::String(vowels[baseVowel]);
            else if (blend > 0.9)
            {
                int nextVowel = juce::jlimit(0, 4, baseVowel + 1);
                return juce::String(vowels[nextVowel]);
            }
            else
            {
                int nextVowel = juce::jlimit(0, 4, baseVowel + 1);
                return juce::String(vowels[baseVowel]) + "-" + juce::String(vowels[nextVowel]);
            }
        }
        else
        {
            // Default: clean decimal
            return juce::String(value, 2);
        }
    };
    
    knob.updateText();
}

void ISODRONEAudioProcessorEditor::setupLabel(juce::Label& label, const juce::String& text, juce::Component& attachTo)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(10.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    label.attachToComponent(&attachTo, false);
    addAndMakeVisible(label);
}

void ISODRONEAudioProcessorEditor::setupSectionLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(14.0f, juce::Font::bold));
    label.setColour(juce::Label::textColourId, juce::Colour(0xff4a9eff));
    addAndMakeVisible(label);
}