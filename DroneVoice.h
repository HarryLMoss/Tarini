# Author: Harry Moss
# Date: 29.05.2024

#pragma once

#include <JuceHeader.h>

class DroneVoice
{
public:
    void prepare(double sampleRate);

    void setFrequency(float newFrequency);
    void setGain(float newGain);
    void setPan(float newPan);

    float process();

private:
    double currentSampleRate = 44100.0;

    float frequency = 220.0f;
    float gain = 0.2f;
    float pan = 0.0f;

    float phase = 0.0f;
    float lfoPhase = 0.0f;

    juce::dsp::StateVariableTPTFilter<float> filter;
};
