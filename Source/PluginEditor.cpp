#include "PluginEditor.h"

namespace
{
constexpr std::array<const char*, 6> paramIds = {"emotion", "energy", "tension", "confidence", "intimacy", "expression"};
constexpr std::array<const char*, 6> paramNames = {"EMOTION", "ENERGY", "TENSION", "CONFIDENCE", "INTIMACY", "EXPRESSION"};
const juce::Colour ink = juce::Colour::fromRGB(226, 56, 48);
}

void EmotionLabAudioProcessorEditor::LineFaderLookAndFeel::drawLinearSlider(
    juce::Graphics& g, int x, int y, int width, int height, float sliderPos,
    float, float, juce::Slider::SliderStyle, juce::Slider&)
{
    // User reference: one thin uninterrupted vertical red stroke, hollow circular handle.
    const float cx = x + width * 0.5f;
    const float top = static_cast<float>(y) + 7.0f;
    const float bottom = static_cast<float>(y + height) - 7.0f;
    const float cy = juce::jlimit(top, bottom, sliderPos);
    g.setColour(ink);
    g.drawLine(cx, top, cx, bottom, 4.0f);
    g.setColour(juce::Colours::white);
    g.fillEllipse(cx - 13.0f, cy - 13.0f, 26.0f, 26.0f);
    g.setColour(ink);
    g.drawEllipse(cx - 13.0f, cy - 13.0f, 26.0f, 26.0f, 4.0f);
    // Pass the red stroke through the centre, matching the sketch.
    g.drawLine(cx, cy - 11.0f, cx, cy + 11.0f, 3.0f);
}

EmotionLabAudioProcessorEditor::EmotionLabAudioProcessorEditor(EmotionLabAudioProcessor& p)
    : juce::AudioProcessorEditor(&p), processor(p)
{
    for (size_t i = 0; i < faders.size(); ++i)
    {
        auto& f = faders[i];
        f.setSliderStyle(juce::Slider::LinearVertical);
        f.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        f.setLookAndFeel(&faderLook);
        f.setDoubleClickReturnValue(true, 50.0);
        f.setScrollWheelEnabled(true);
        addAndMakeVisible(f);

        names[i].setText(paramNames[i], juce::dontSendNotification);
        names[i].setJustificationType(juce::Justification::centred);
        names[i].setFont(juce::Font(juce::FontOptions(10.0f).withStyle("Bold")));
        names[i].setColour(juce::Label::textColourId, juce::Colours::black);
        addAndMakeVisible(names[i]);

        values[i].setJustificationType(juce::Justification::centred);
        values[i].setFont(juce::Font(juce::FontOptions(12.0f)));
        values[i].setColour(juce::Label::textColourId, juce::Colour::fromRGB(80, 80, 80));
        addAndMakeVisible(values[i]);

        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.apvts, paramIds[i], f);
        const size_t index = i;
        f.onValueChange = [this, index] {
            values[index].setText(juce::String(juce::roundToInt(faders[index].getValue())),
                                  juce::dontSendNotification);
        };
        f.onValueChange();
    }
    setSize(720, 410);
}

EmotionLabAudioProcessorEditor::~EmotionLabAudioProcessorEditor()
{
    for (auto& f : faders) f.setLookAndFeel(nullptr);
}

void EmotionLabAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::white);
    g.setColour(juce::Colours::black);
    g.setFont(juce::Font(juce::FontOptions(16.0f).withStyle("Bold")));
    g.drawText("EMOTION LAB", 26, 17, getWidth() - 52, 23, juce::Justification::centredLeft);
    g.setColour(juce::Colour::fromRGB(145,145,145));
    g.setFont(juce::Font(juce::FontOptions(10.0f)));
    g.drawText("0.2  /  SIX FADERS", getWidth()-170, 21, 146, 18, juce::Justification::centredRight);
}

void EmotionLabAudioProcessorEditor::resized()
{
    constexpr int margin = 30;
    const int usable = getWidth() - 2 * margin;
    const int step = usable / 6;
    for (int i = 0; i < 6; ++i)
    {
        int cx = margin + step * i + step / 2;
        faders[static_cast<size_t>(i)].setBounds(cx - 26, 74, 52, 248);
        names[static_cast<size_t>(i)].setBounds(cx - step / 2, 332, step, 23);
        values[static_cast<size_t>(i)].setBounds(cx - 23, 359, 46, 22);
    }
}
