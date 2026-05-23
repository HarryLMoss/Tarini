/*
===============================================================================

    DroneVoice.cpp
    Author: Harry Moss
    Created: 29th May 2024

    Realtime drone voice DSP implementation for Tarini.

    Implements one voice of the Tanpura drone:
      - Additive synthesis: fundamental + 2nd and 3rd harmonics
      - LFO pitch modulation for organic string-like intonation drift
      - Bandpass filter to simulate instrument body resonance
      - Stereo output handled upstream in TariniAudioProcessor (this class
        produces one mono sample per call to process())

===============================================================================
*/

#include "DroneVoice.h"

// ─────────────────────────────────────────────────────────────────────────────
// prepare()
//
// Called by TariniAudioProcessor::prepareToPlay() before audio starts.
// This is the correct place for any setup that depends on sample rate.
// Nothing in process() should allocate memory or call OS functions.
// ─────────────────────────────────────────────────────────────────────────────
void DroneVoice::prepare(double sampleRate)
{
    // Store the sample rate so process() can convert Hz to radians/sample.
    currentSampleRate = sampleRate;

    // ProcessSpec tells JUCE DSP objects about the audio environment before
    // they process any audio. All three fields must be set.
    juce::dsp::ProcessSpec spec;
    spec.sampleRate      = sampleRate;
    spec.maximumBlockSize = 512;  // largest buffer we will ever pass
    spec.numChannels      = 1;    // this voice is mono

    // Initialise the filter's internal state and allocate any internal buffers.
    // Must be called before processSample() is ever called.
    filter.prepare(spec);

    // Configure as a bandpass filter. The StateVariableTPTFilter supports
    // lowpass, highpass, and bandpass modes — bandpass passes a band of
    // frequencies around the cutoff and attenuates everything outside it.
    filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);

    // Cutoff at 900 Hz models the resonant peak of a Tanpura body.
    // Frequencies near 900 Hz are emphasised; others are attenuated.
    filter.setCutoffFrequency(900.0f);

    // Resonance (Q factor) of 0.4 gives a gentle, wide peak — appropriate
    // for body resonance simulation. Higher Q = narrower, more pronounced peak.
    filter.setResonance(0.4f);

    // ── LFO rate calculation ──────────────────────────────────────────────
    //
    // We want an LFO at approximately 0.1 Hz (one full cycle every 10 seconds).
    // To advance the LFO phase accumulator by the right amount each sample,
    // we convert the frequency in Hz to radians per sample:
    //
    //   increment = (frequency_Hz / sampleRate) * 2π
    //
    // This makes the LFO rate sample-rate-independent: it will be 0.1 Hz
    // whether the host runs at 44100, 48000, or 96000 Hz.
    // The cast to float is explicit to avoid implicit double → float warnings.
    lfoIncrement =
        (0.1f / static_cast<float>(sampleRate))
        * juce::MathConstants<float>::twoPi;
}

// ─────────────────────────────────────────────────────────────────────────────
// setFrequency() / setGain()
//
// Simple setters called from TariniAudioProcessor::updateVoices() at the
// start of each processBlock. They are not called mid-sample, so there is
// no risk of a click from a discontinuous frequency change within a block.
// ─────────────────────────────────────────────────────────────────────────────
void DroneVoice::setFrequency(float f) { frequency = f; }
void DroneVoice::setGain(float g)      { gain = g; }

// ─────────────────────────────────────────────────────────────────────────────
// process()
//
// Called once per sample from TariniAudioProcessor::processBlock().
// Computes and returns one mono output sample.
//
// Realtime safety: no allocation, no locks, no OS calls, no branching on
// shared state. All reads are from local member variables only.
// ─────────────────────────────────────────────────────────────────────────────
float DroneVoice::process()
{
    // ── LFO: pitch modulation ─────────────────────────────────────────────
    //
    // The LFO produces a sinusoidal value in [-0.5, +0.5] Hz range.
    // This is added directly to the frequency in Hz, creating subtle
    // pitch drift that simulates the natural intonation variation of a
    // plucked string — the "jiva" shimmer characteristic of the Tanpura.
    float lfo     = std::sin(lfoPhase) * 0.5f;
    float modFreq = frequency + lfo;

    // ── Main oscillator phase increment ───────────────────────────────────
    //
    // Convert the (LFO-modulated) frequency from Hz to radians per sample.
    // phaseInc tells us how much to advance the phase accumulator each sample
    // so that the oscillator completes exactly `modFreq` full cycles per second.
    float phaseInc =
        (modFreq / static_cast<float>(currentSampleRate))
        * juce::MathConstants<float>::twoPi;

    // Advance the main oscillator phase.
    phase += phaseInc;

    // Wrap phase to [0, 2π). Without this, the float would grow without bound
    // over time, eventually losing precision and producing a detuned or silent
    // output. Subtraction is preferred over fmod() for performance.
    if (phase > juce::MathConstants<float>::twoPi)
        phase -= juce::MathConstants<float>::twoPi;

    // Advance the LFO phase and wrap it the same way.
    lfoPhase += lfoIncrement;

    if (lfoPhase > juce::MathConstants<float>::twoPi)
        lfoPhase -= juce::MathConstants<float>::twoPi;

    // ── Additive synthesis ────────────────────────────────────────────────
    //
    // A pure sine wave sounds thin and electronic. Real instruments produce
    // a fundamental frequency plus additional harmonics (integer multiples).
    // We add the 2nd and 3rd harmonics — phase * 2 and phase * 3 — at
    // reduced amplitudes to model the overtone structure of a plucked string.
    //
    // This technique is called additive synthesis: building a complex timbre
    // by summing sinusoids at harmonic frequencies.
    //
    //   Fundamental (1st harmonic):  std::sin(phase)        — 100% amplitude
    //   2nd harmonic (octave up):    std::sin(phase * 2.0f) — 30% amplitude
    //   3rd harmonic (octave + 5th): std::sin(phase * 3.0f) — 15% amplitude
    //
    // The 2nd harmonic is an even harmonic — adding even harmonics creates
    // an asymmetric waveform, similar to the asymmetric pluck of a real string.
    float s = std::sin(phase);
    s += 0.3f  * std::sin(phase * 2.0f);
    s += 0.15f * std::sin(phase * 3.0f);

    // ── Bandpass filter ───────────────────────────────────────────────────
    //
    // Pass the additive signal through the bandpass filter configured in
    // prepare() — centred at 900 Hz to simulate body resonance.
    // processSample(channel, sample): channel 0 because this is a mono voice.
    // The result is scaled by this voice's gain before returning.
    return filter.processSample(0, s) * gain;
}