#pragma once
#include <JuceHeader.h>
#include <array>

class EmotionLabAudioProcessor final : public juce::AudioProcessor
{
public:
    EmotionLabAudioProcessor();
    ~EmotionLabAudioProcessor() override = default;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "EMOTION LAB"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;

    juce::AudioProcessorValueTreeState apvts;
private:
    struct ChannelState
    {
        // Two inexpensive first-order splitters used for smooth spectral tilt.
        float low = 0.0f, high = 0.0f, envelope = 0.0f, prevInput = 0.0f;
    };
    std::array<ChannelState, 2> channels{};
    double sr = 44100.0;
    float lfoPhase = 0.0f;
    std::array<juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear>, 6> smooth;
    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EmotionLabAudioProcessor)
};
