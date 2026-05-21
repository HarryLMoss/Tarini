/*
===============================================================================

    PluginEditor.cpp
    Author: Harry Moss
    Created: 21st May 2026

    GUI editor implementation for Tarini plugin.

===============================================================================
*/

#include "PluginEditor.h"

TariniEditor::TariniEditor(TariniAudioProcessor& p)
    : AudioProcessorEditor(&p),
      proc(p)
{
    tonicSlider.setSliderStyle(juce::Slider::Rotary);
    tonicSlider.setTextBoxStyle(
        juce::Slider::TextBoxBelow,
        false,
        80,
        20);

    gainSlider.setSliderStyle(juce::Slider::Rotary);
    gainSlider.setTextBoxStyle(
        juce::Slider::TextBoxBelow,
        false,
        80,
        20);

    tonicAttach = std::make_unique<Attach>(
        proc.apvts,
        "tonic",
        tonicSlider);

    gainAttach = std::make_unique<Attach>(
        proc.apvts,
        "gain",
        gainSlider);

    addAndMakeVisible(tonicSlider);
    addAndMakeVisible(gainSlider);

    setSize(400, 300);
}

void TariniEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);

    g.setFont(20.0f);

    g.drawFittedText(
        "Tarini",
        getLocalBounds(),
        juce::Justification::centredTop,
        1);
}

void TariniEditor::resized()
{
    tonicSlider.setBounds(40, 80, 140, 140);
    gainSlider.setBounds(220, 80, 140, 140);
}