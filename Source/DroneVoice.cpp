/*
===============================================================================

    DroneVoice.cpp
    Author: Harry Moss
    Created: 29th May 2024

    Realtime drone voice DSP implementation for Tarini.

===============================================================================
*/

#include "DroneVoice.h"

void DroneVoice::prepare(double sampleRate)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = 512;
    spec.numChannels = 1;

    filter.prepare(spec);

    filter.setType(
        juce::dsp::StateVariableTPTFilterType::bandpass);

    filter.setCutoffFrequency(900.0f);
    filter.setResonance(0.4f);

    // ~0.1Hz sample-rate-independent LFO
    lfoIncrement =
        (0.1f / static_cast<float>(sampleRate))
        * juce::MathConstants<float>::twoPi;
}

void DroneVoice::setFrequency(float f)
{
    frequency = f;
}

void DroneVoice::setGain(float g)
{
    gain = g;
}

float DroneVoice::process()
{
    float lfo = std::sin(lfoPhase) * 0.5f;

    float modFreq = frequency + lfo;

    float phaseInc =
        (modFreq / static_cast<float>(currentSampleRate))
        * juce::MathConstants<float>::twoPi;

    phase += phaseInc;

    if (phase > juce::MathConstants<float>::twoPi)
        phase -= juce::MathConstants<float>::twoPi;

    lfoPhase += lfoIncrement;

    if (lfoPhase > juce::MathConstants<float>::twoPi)
        lfoPhase -= juce::MathConstants<float>::twoPi;

    float s = std::sin(phase);

    s += 0.3f * std::sin(phase * 2.0f);
    s += 0.15f * std::sin(phase * 3.0f);
    
    return filter.processSample(0, s) * gain;
}