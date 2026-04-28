// ==========================================================
// Author: Harry Moss
// Date: 29.05.2024
// ==========================================================

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
}

void DroneVoice::setFrequency(float newFrequency)
{
    frequency = newFrequency;
}

void DroneVoice::setGain(float newGain)
{
    gain = newGain;
}

float DroneVoice::process()
{
    // Organic pitch drift

    float lfo = std::sin(lfoPhase) * 0.5f;

    float modulatedFrequency = frequency + lfo;

    float phaseIncrement =
        (modulatedFrequency / (float) currentSampleRate)
        * juce::MathConstants<float>::twoPi;

    phase += phaseIncrement;

    if (phase > juce::MathConstants<float>::twoPi)
        phase -= juce::MathConstants<float>::twoPi;

    lfoPhase += 0.00003f;

    if (lfoPhase > juce::MathConstants<float>::twoPi)
        lfoPhase -= juce::MathConstants<float>::twoPi;

    // Additive harmonic synthesis

    float sample = std::sin(phase);

    sample += 0.3f * std::sin(phase * 2.0f);
    sample += 0.15f * std::sin(phase * 3.0f);

    // Resonant filtering

    sample = filter.processSample(sample);

    return sample * gain;
}
