/*
===============================================================================

    PluginProcessor.cpp
    Author: Harry Moss
    Created: 21st May 2026

    Main audio processor implementation for Tarini.
    Handles DSP processing, parameter management,
    and plugin lifecycle behaviour.

===============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

TariniAudioProcessor::TariniAudioProcessor()
    : AudioProcessor(
        BusesProperties()
            .withOutput(
                "Output",
                juce::AudioChannelSet::stereo(),
                true)),
      apvts(*this,
            nullptr,
            "PARAMETERS",
            makeLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
TariniAudioProcessor::makeLayout()
{
    using namespace juce;

    AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{"tonic", 1},
        "Tonic",
        NormalisableRange<float>(110.0f, 440.0f, 0.1f),
        146.83f));

    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{"gain", 1},
        "Master Gain",
        NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.3f));

    return layout;
}

void TariniAudioProcessor::prepareToPlay(
    double sampleRate,
    int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);

    for (auto& voice : voices)
        voice.prepare(sampleRate);

    updateVoices();
}

void TariniAudioProcessor::updateVoices()
{
    float tonic =
        apvts.getRawParameterValue("tonic")->load();

    voices[0].setFrequency(tonic);
    voices[1].setFrequency(tonic * 1.5f);
    voices[2].setFrequency(tonic * 2.0f);
    voices[3].setFrequency(tonic * 2.0f * 1.01f);

    voices[0].setGain(0.20f);
    voices[1].setGain(0.18f);
    voices[2].setGain(0.15f);
    voices[3].setGain(0.15f);
}

void TariniAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    updateVoices();

    auto* left =
        buffer.getWritePointer(0);

    auto* right =
        buffer.getWritePointer(1);

    const int numSamples =
        buffer.getNumSamples();

    float masterGain =
        apvts.getRawParameterValue("gain")->load();

    for (int sample = 0;
         sample < numSamples;
         ++sample)
    {
        float s0 = voices[0].process();
        float s1 = voices[1].process();
        float s2 = voices[2].process();
        float s3 = voices[3].process();

        float leftMix =
            (s0 * 0.8f +
             s1 * 0.3f +
             s2 * 0.7f +
             s3 * 0.2f)
            * masterGain;

        float rightMix =
            (s0 * 0.2f +
             s1 * 0.7f +
             s2 * 0.3f +
             s3 * 0.8f)
            * masterGain;

        left[sample] = leftMix;
        right[sample] = rightMix;
    }
}

juce::AudioProcessorEditor*
TariniAudioProcessor::createEditor()
{
    return new TariniEditor(*this);
}

void TariniAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData)
{
    if (auto xml =
            apvts.copyState().createXml())
    {
        copyXmlToBinary(*xml, destData);
    }
}

void TariniAudioProcessor::setStateInformation(
    const void* data,
    int sizeInBytes)
{
    if (auto xml =
            getXmlFromBinary(
                data,
                sizeInBytes))
    {
        apvts.replaceState(
            juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE
createPluginFilter()
{
    return new TariniAudioProcessor();
}