#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>
#include <memory>

class EmotionLabAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit EmotionLabAudioProcessorEditor(EmotionLabAudioProcessor&);
    ~EmotionLabAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    class LineFaderLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                              float sliderPos, float minSliderPos, float maxSliderPos,
                              juce::Slider::SliderStyle, juce::Slider&) override;
    } faderLook;

    EmotionLabAudioProcessor& processor;
    std::array<juce::Slider, 6> faders;
    std::array<juce::Label, 6> names;
    std::array<juce::Label, 6> values;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 6> attachments;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EmotionLabAudioProcessorEditor)
};
