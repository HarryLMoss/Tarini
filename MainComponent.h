#pragma once

#include <JuceHeader.h>
#include "DroneVoice.h"

class MainComponent : public juce::AudioAppComponent,
                      public juce::Slider::Listener
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected,
                       double sampleRate) override;

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void sliderValueChanged(juce::Slider* slider) override;

private:
    DroneVoice string1;
    DroneVoice string2;
    DroneVoice string3;
    DroneVoice string4;

    juce::Slider tonicSlider;
    juce::Slider gainSlider;

    juce::Label tonicLabel;
    juce::Label gainLabel;

    float masterGain = 0.3f;

    void updateFrequencies();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
