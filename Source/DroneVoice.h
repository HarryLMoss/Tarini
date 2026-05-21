/*
===============================================================================

    DroneVoice.h
    Author: Harry Moss
    Created: 29th May 2024

    Realtime drone voice DSP engine for Tarini.
    Implements additive synthesis, harmonic partials,
    modulation, and resonance filtering.

===============================================================================
*/

#pragma once

#include <JuceHeader.h>

class DroneVoice
{
public:
    DroneVoice() = default;

    void prepare(double sampleRate);
    void setFrequency(float f);
    void setGain(float g);
    float process();

private:
    float frequency = 220.0f;
    float gain = 0.2f;

    float phase = 0.0f;
    float lfoPhase = 0.0f;
    float lfoIncrement = 0.0f;

    double currentSampleRate = 44100.0;

    juce::dsp::StateVariableTPTFilter<float> filter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DroneVoice)
};