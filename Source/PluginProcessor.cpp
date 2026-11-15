#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
constexpr std::array<const char*, 6> ids = {"emotion", "energy", "tension", "confidence", "intimacy", "expression"};
constexpr std::array<const char*, 6> names = {"01 Emotion  Sad / Happy", "02 Energy  Calm / Excited", "03 Tension  Relaxed / Tense", "04 Confidence  Unsure / Confident", "05 Intimacy  Distant / Close", "06 Expression  Restrained / Expressive"};
float bipolar(float value) { return (value - 50.0f) * 0.02f; }
}

juce::AudioProcessorValueTreeState::ParameterLayout EmotionLabAudioProcessor::makeLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (size_t i = 0; i < ids.size(); ++i)
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID(ids[i], 1), names[i],
            juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 50.0f));
    return layout;
}

EmotionLabAudioProcessor::EmotionLabAudioProcessor()
    : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts(*this, nullptr, "PARAMETERS", makeLayout())
{
}

bool EmotionLabAudioProcessor::isBusesLayoutSupported(const BusesLayout& layout) const
{
    const auto out = layout.getMainOutputChannelSet();
    return (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo())
        && layout.getMainInputChannelSet() == out;
}

void EmotionLabAudioProcessor::prepareToPlay(double sampleRate, int)
{
    sr = juce::jmax(8000.0, sampleRate);
    channels = {};
    lfoPhase = 0.0f;
    for (size_t i = 0; i < ids.size(); ++i)
    {
        smooth[i].reset(sr, 0.035);
        smooth[i].setCurrentAndTargetValue(apvts.getRawParameterValue(ids[i])->load());
    }
}

void EmotionLabAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int count = juce::jmin(2, buffer.getNumChannels());
    for (int c = count; c < buffer.getNumChannels(); ++c)
        buffer.clear(c, 0, buffer.getNumSamples());
    for (size_t i = 0; i < ids.size(); ++i)
        smooth[i].setTargetValue(apvts.getRawParameterValue(ids[i])->load());

    // These are audible DSP proxies for perceived emotional qualities, NOT speech emotion conversion.
    // All processing is allocation-free on the audio thread.
    for (int n = 0; n < buffer.getNumSamples(); ++n)
    {
        const float emotion = bipolar(smooth[0].getNextValue());
        const float energy = bipolar(smooth[1].getNextValue());
        const float tension = bipolar(smooth[2].getNextValue());
        const float confidence = bipolar(smooth[3].getNextValue());
        const float intimacy = bipolar(smooth[4].getNextValue());
        const float expression = bipolar(smooth[5].getNextValue());

        // At all-neutral settings, the effect is a bit-perfect passthrough.
        const float lowCoeff = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * 360.0f / (float) sr);
        const float highCoeff = 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * 3100.0f / (float) sr);
        const float bright = 0.40f * emotion + 0.15f * tension + 0.25f * intimacy;
        const float bass = 0.20f * confidence + 0.12f * intimacy;
        const float drive = 1.0f + 4.0f * juce::jmax(0.0f, energy) + 2.0f * juce::jmax(0.0f, tension);
        const float compress = juce::jmax(0.0f, confidence) * 0.55f;
        const float tremDepth = std::abs(expression) * 0.11f;
        const float tremRate = 1.7f + 3.0f * juce::jmax(0.0f, expression);
        lfoPhase += 2.0f * juce::MathConstants<float>::pi * tremRate / (float) sr;
        if (lfoPhase >= 2.0f * juce::MathConstants<float>::pi)
            lfoPhase -= 2.0f * juce::MathConstants<float>::pi;
        const float trem = 1.0f - tremDepth * (0.5f - 0.5f * std::sin(lfoPhase));
        const float gain = juce::Decibels::decibelsToGain(2.0f * energy + 1.2f * confidence - 0.8f * tension);

        for (int c = 0; c < count; ++c)
        {
            float* data = buffer.getWritePointer(c);
            auto& s = channels[(size_t)c];
            const float input = data[n];
            s.low += lowCoeff * (input - s.low);
            s.high += highCoeff * (input - s.high);
            const float hi = input - s.high;
            float y = input + bass * s.low + bright * hi;
            const float saturated = std::tanh(y * drive) / std::tanh(drive);
            const float saturationMix = juce::jlimit(0.0f, 0.8f, 0.34f * juce::jmax(0.0f, energy) + 0.22f * juce::jmax(0.0f, tension));
            y = y + (saturated - y) * saturationMix;
            s.envelope += (std::abs(y) - s.envelope) * (std::abs(y) > s.envelope ? 0.013f : 0.0008f);
            y *= 1.0f / (1.0f + compress * s.envelope * 2.8f);
            // Negative energy reduces dynamics by a gentle fixed gain rather than gating speech.
            y *= gain * trem;
            data[n] = juce::jlimit(-1.25f, 1.25f, y);
        }
    }
}

juce::AudioProcessorEditor* EmotionLabAudioProcessor::createEditor()
{
    return new EmotionLabAudioProcessorEditor(*this);
}

void EmotionLabAudioProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml()) copyXmlToBinary(*xml, dest);
}

void EmotionLabAudioProcessor::setStateInformation(const void* bytes, int size)
{
    if (auto xml = getXmlFromBinary(bytes, size))
        if (xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new EmotionLabAudioProcessor(); }
