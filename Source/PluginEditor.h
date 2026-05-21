/*
===============================================================================

    PluginEditor.h
    Author: Harry Moss
    Created: 21st May 2026

    GUI editor for Tarini plugin.

===============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class TariniEditor : public juce::AudioProcessorEditor
{
public:
    explicit TariniEditor(TariniAudioProcessor&);
    ~TariniEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    using Attach =
        juce::AudioProcessorValueTreeState::SliderAttachment;

    TariniAudioProcessor& proc;

    juce::Slider tonicSlider;
    juce::Slider gainSlider;

    juce::Label tonicLabel;
    juce::Label gainLabel;

    std::unique_ptr<Attach> tonicAttach;
    std::unique_ptr<Attach> gainAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        TariniEditor)
};