# Author: Harry Moss
# Date: 29.05.2024

#include "MainComponent.h"

MainComponent::MainComponent()
{
    setSize(600, 300);

    tonicSlider.setRange(110.0, 440.0, 1.0);
    tonicSlider.setValue(146.83);
    tonicSlider.addListener(this);
    tonicSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,
                                false,
                                80,
                                20);
    addAndMakeVisible(tonicSlider);

    tonicLabel.setText("Tonic Frequency", juce::dontSendNotification);
    addAndMakeVisible(tonicLabel);

    gainSlider.setRange(0.0, 1.0, 0.01);
    gainSlider.setValue(0.3);
    gainSlider.addListener(this);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxBelow,
                               false,
                               80,
                               20);
    addAndMakeVisible(gainSlider);

    gainLabel.setText("Master Gain", juce::dontSendNotification);
    addAndMakeVisible(gainLabel);

    setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int, double sampleRate)
{
    string1.prepare(sampleRate);
    string2.prepare(sampleRate);
    string3.prepare(sampleRate);
    string4.prepare(sampleRate);

    updateFrequencies();
}

void MainComponent::releaseResources()
{
}

void MainComponent::updateFrequencies()
{
    float tonic = (float) tonicSlider.getValue();

    string1.setFrequency(tonic);
    string2.setFrequency(tonic * 1.5f);
    string3.setFrequency(tonic * 2.0f);
    string4.setFrequency(tonic * 2.0f * 1.01f);

    string1.setGain(0.20f);
    string2.setGain(0.18f);
    string3.setGain(0.15f);
    string4.setGain(0.15f);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* leftBuffer = bufferToFill.buffer->getWritePointer(0,
                                                            bufferToFill.startSample);

    auto* rightBuffer = bufferToFill.buffer->getWritePointer(1,
                                                             bufferToFill.startSample);

    for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
    {
        float s1 = string1.process();
        float s2 = string2.process();
        float s3 = string3.process();
        float s4 = string4.process();

        float left =
            (s1 * 0.8f) +
            (s2 * 0.3f) +
            (s3 * 0.7f) +
            (s4 * 0.2f);

        float right =
            (s1 * 0.2f) +
            (s2 * 0.7f) +
            (s3 * 0.3f) +
            (s4 * 0.8f);

        left *= masterGain;
        right *= masterGain;

        leftBuffer[sample] = left;
        rightBuffer[sample] = right;
    }
}

void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &tonicSlider)
    {
        updateFrequencies();
    }

    if (slider == &gainSlider)
    {
        masterGain = (float) gainSlider.getValue();
    }
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::orange);
    g.setFont(28.0f);

    g.drawFittedText("Tarini",
                     getLocalBounds().removeFromTop(60),
                     juce::Justification::centred,
                     1);

    g.setFont(16.0f);
    g.drawFittedText("Tanpura-inspired Generative Drone Engine",
                     0,
                     50,
                     getWidth(),
                     30,
                     juce::Justification::centred,
                     1);
}

void MainComponent::resized()
{
    tonicLabel.setBounds(50, 100, 200, 20);
    tonicSlider.setBounds(50, 130, 220, 60);

    gainLabel.setBounds(320, 100, 200, 20);
    gainSlider.setBounds(320, 130, 220, 60);
}
