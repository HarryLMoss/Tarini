/*
===============================================================================

    PluginProcessor.h
    Author: Harry Moss
    Created: 21st May 2026

    Main audio processor for Tarini.
    Handles DSP processing, parameter state,
    and plugin lifecycle management.

===============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DroneVoice.h"

class TariniAudioProcessor : public juce::AudioProcessor
{
public:
    TariniAudioProcessor();
    ~TariniAudioProcessor() override = default;

    void prepareToPlay(double sampleRate,
                       int samplesPerBlock) override;

    void releaseResources() override {}

    void processBlock(juce::AudioBuffer<float>&,
                      juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;

    bool hasEditor() const override { return true; }

    const juce::String getName() const override
    {
        return "Tarini";
    }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }

    double getTailLengthSeconds() const override
    {
        return 0.0;
    }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }

    void setCurrentProgram(int) override {}

    const juce::String getProgramName(int) override
    {
        return {};
    }

    void changeProgramName(
        int,
        const juce::String&) override {}

    void getStateInformation(
        juce::MemoryBlock&) override;

    void setStateInformation(
        const void*,
        int) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout
    makeLayout();

    void updateVoices();

    DroneVoice voices[4];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        TariniAudioProcessor)
};