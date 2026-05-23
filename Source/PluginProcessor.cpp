/*
===============================================================================

    PluginProcessor.cpp
    Author: Harry Moss
    Created: 21st May 2026

    Main audio processor implementation for Tarini.
    Handles DSP processing, parameter management,
    and plugin lifecycle behaviour.

===============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
//
// The member initialiser list runs before the constructor body.
// Two things happen here:
//
// 1. AudioProcessor base class is initialised with a BusesProperties object
//    that declares this plugin has one stereo output and no inputs.
//    (It is a generator/synth, not an effect.)
//
// 2. apvts is initialised. APVTS requires a reference to *this (the processor),
//    an optional UndoManager (nullptr = no undo), a state identifier string,
//    and the parameter layout returned by makeLayout().
//    makeLayout() is static so it can be called before *this is fully
//    constructed.
// ─────────────────────────────────────────────────────────────────────────────
TariniAudioProcessor::TariniAudioProcessor()
    : AudioProcessor(
        BusesProperties()
            .withOutput(
                "Output",
                juce::AudioChannelSet::stereo(),
                true)),          // true = enabled by default
      apvts(*this,
            nullptr,             // no UndoManager
            "PARAMETERS",        // ValueTree identifier for this state
            makeLayout())
{
}

// ─────────────────────────────────────────────────────────────────────────────
// makeLayout()
//
// Defines all plugin parameters and their ranges/defaults.
// Returns a ParameterLayout which APVTS uses to create parameter objects.
//
// Each AudioParameterFloat takes:
//   - ParameterID { "id_string", version_hint }
//   - Display name (shown in DAW automation lanes)
//   - NormalisableRange (min, max, step size)
//   - Default value
// ─────────────────────────────────────────────────────────────────────────────
juce::AudioProcessorValueTreeState::ParameterLayout
TariniAudioProcessor::makeLayout()
{
    using namespace juce;

    AudioProcessorValueTreeState::ParameterLayout layout;

    // Tonic frequency: the fundamental pitch of the SA string.
    // Range 110–440 Hz covers A2 to A4 — the practical range of a Tanpura.
    // Default 146.83 Hz = D3 (re below middle C), a common Tanpura tuning.
    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{"tonic", 1},          // "1" = version hint for preset compat
        "Tonic",
        NormalisableRange<float>(110.0f, 440.0f, 0.1f),
        146.83f));                         // D3 default

    // Master gain: overall output level scalar.
    // Range 0.0–1.0, step 0.001 for fine control.
    // Default 0.3 prevents clipping when all four voices sum together.
    layout.add(std::make_unique<AudioParameterFloat>(
        ParameterID{"gain", 1},
        "Master Gain",
        NormalisableRange<float>(0.0f, 1.0f, 0.001f),
        0.3f));

    return layout;
}

// ─────────────────────────────────────────────────────────────────────────────
// prepareToPlay()
//
// Called by the host before audio starts (or when sample rate / block size
// changes). This is where all sample-rate-dependent initialisation happens.
//
// Critical rule: never allocate memory, resize buffers, or do expensive
// computation inside processBlock(). Do it here instead.
// ─────────────────────────────────────────────────────────────────────────────
void TariniAudioProcessor::prepareToPlay(
    double sampleRate,
    int samplesPerBlock)
{
    // samplesPerBlock is not used directly — each DroneVoice configures its
    // own maximumBlockSize internally. ignoreUnused suppresses the compiler
    // warning without adding a cast.
    juce::ignoreUnused(samplesPerBlock);

    // Initialise all four voices with the host's sample rate.
    // This configures the filter, computes the LFO increment, and resets
    // all phase accumulators.
    for (auto& voice : voices)
        voice.prepare(sampleRate);

    // Set initial frequencies and gains based on current parameter values,
    // so the first processBlock produces the correct output immediately.
    updateVoices();
}

// ─────────────────────────────────────────────────────────────────────────────
// updateVoices()
//
// Reads the current tonic value from APVTS and assigns frequencies and gains
// to all four voices to mirror authentic Tanpura string tuning.
//
// Called from prepareToPlay() and at the top of every processBlock().
// The atomic load is safe on the audio thread — APVTS stores parameters as
// std::atomic<float> internally.
// ─────────────────────────────────────────────────────────────────────────────
void TariniAudioProcessor::updateVoices()
{
    // getRawParameterValue returns a pointer to the internal std::atomic<float>.
    // .load() performs an atomic read — thread-safe, no lock needed.
    float tonic =
        apvts.getRawParameterValue("tonic")->load();

    // ── Tanpura string tuning ─────────────────────────────────────────────
    //
    // Traditional 4-string Tanpura tuning:
    //   String 1: SA  — the tonic (fundamental)
    //   String 2: PA  — the perfect fifth, ratio 3:2 (tonic × 1.5)
    //   String 3: SA' — the upper octave, ratio 2:1 (tonic × 2.0)
    //   String 4: SA' — upper octave, very slightly sharp (tonic × 2.0 × 1.01)
    //
    // The 1% detune on string 4 creates beating between strings 3 and 4 —
    // this is the "jiva" shimmer, the defining acoustic character of the
    // Tanpura. Without this, the drone sounds static and electronic.
    voices[0].setFrequency(tonic);             // SA
    voices[1].setFrequency(tonic * 1.5f);      // PA
    voices[2].setFrequency(tonic * 2.0f);      // SA'
    voices[3].setFrequency(tonic * 2.0f * 1.01f); // SA' detuned

    // ── Per-voice gain levels ─────────────────────────────────────────────
    //
    // The SA fundamental (voice 0) is the loudest — it is the tonal anchor.
    // PA (voice 1) slightly quieter — in a real Tanpura it is plucked less
    // forcefully.
    // The two upper SA' voices (2 and 3) are quieter still — they provide
    // shimmer and harmonics rather than fundamental weight.
    voices[0].setGain(0.20f);  // SA  — loudest
    voices[1].setGain(0.18f);  // PA
    voices[2].setGain(0.15f);  // SA'
    voices[3].setGain(0.15f);  // SA' detuned
}

// ─────────────────────────────────────────────────────────────────────────────
// processBlock()
//
// The realtime audio callback. Called by the host every ~5-12ms.
// Fills `buffer` with one block of stereo output audio.
//
// Realtime constraints (strictly enforced):
//   ✗ No memory allocation (new, delete, malloc, vector::push_back, etc.)
//   ✗ No mutex locks or blocking synchronisation primitives
//   ✗ No file I/O, network calls, or system calls (printf, etc.)
//   ✓ Read parameters via atomic load only
//   ✓ All DSP objects pre-allocated in prepareToPlay
// ─────────────────────────────────────────────────────────────────────────────
void TariniAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer&)           // MidiBuffer ignored — Tarini produces no MIDI
{
    // ScopedNoDenormals disables denormal floating-point numbers for the
    // duration of this scope. Denormals are extremely small (near-zero) floats
    // that can cause a 10-100x CPU performance hit on some processor
    // architectures (x86 in particular). Audio signals naturally produce
    // near-zero values during quiet sections, so this guard is standard
    // practice in any realtime audio plugin.
    juce::ScopedNoDenormals noDenormals;

    // Update all voice frequencies from the current APVTS tonic value.
    // This is called every block so that any tonic slider movement by the user
    // (or DAW automation) takes effect at the start of the next block.
    // The latency is at most one block (~12ms at 44100/512) — imperceptible.
    updateVoices();

    // Get raw pointers to the left and right output channel arrays.
    // getWritePointer(channelIndex) returns a float* to the start of the
    // channel's sample data. Writing here directly fills the output buffer.
    auto* left  = buffer.getWritePointer(0);
    auto* right = buffer.getWritePointer(1);

    const int numSamples = buffer.getNumSamples();

    // Read master gain atomically. Safe on the audio thread — APVTS stores
    // this as a std::atomic<float> and load() is a lock-free operation.
    float masterGain =
        apvts.getRawParameterValue("gain")->load();

    // ── Per-sample synthesis loop ─────────────────────────────────────────
    //
    // For each sample in this block, advance all four voices by one sample
    // and mix their outputs into left and right channels.
    for (int sample = 0; sample < numSamples; ++sample)
    {
        // Advance each voice by one sample. Each returns one mono float.
        float s0 = voices[0].process();  // SA
        float s1 = voices[1].process();  // PA
        float s2 = voices[2].process();  // SA'
        float s3 = voices[3].process();  // SA' detuned

        // ── Stereo panning matrix ─────────────────────────────────────────
        //
        // Each voice is panned across the stereo field with a fixed
        // left/right gain pair. The coefficients are chosen to spread the
        // four strings across the stereo image — similar to how the four
        // strings of a physical Tanpura sit at slightly different positions
        // on the instrument body.
        //
        // Voice:    Left  Right   (sums to 1.0 per voice for equal loudness)
        //   SA:     0.8   0.2     — mostly left, the grounding anchor
        //   PA:     0.3   0.7     — mostly right
        //   SA':    0.7   0.3     — slightly left
        //   SA'det: 0.2   0.8     — mostly right, creates wide shimmer
        //
        // masterGain is applied once after mixing, not per-voice, for
        // efficiency.
        float leftMix =
            (s0 * 0.8f +
             s1 * 0.3f +
             s2 * 0.7f +
             s3 * 0.2f)
            * masterGain;

        float rightMix =
            (s0 * 0.2f +
             s1 * 0.7f +
             s2 * 0.3f +
             s3 * 0.8f)
            * masterGain;

        // Write directly into the output buffer arrays.
        left[sample]  = leftMix;
        right[sample] = rightMix;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// createEditor()
//
// Instantiates and returns the GUI. Called by the host on the message thread
// when the user opens the plugin window. The host manages the editor's
// lifetime and calls delete on it when the window closes.
// ─────────────────────────────────────────────────────────────────────────────
juce::AudioProcessorEditor*
TariniAudioProcessor::createEditor()
{
    return new TariniEditor(*this);
}

// ─────────────────────────────────────────────────────────────────────────────
// getStateInformation()
//
// Called by the host when saving a project (or preset). Serialises the full
// APVTS parameter state to an XML string, then encodes it as binary data in
// destData. The host stores this blob in the project file and passes it back
// to setStateInformation() on recall.
// ─────────────────────────────────────────────────────────────────────────────
void TariniAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData)
{
    // copyState() returns a deep copy of the APVTS ValueTree (thread-safe).
    // createXml() converts it to a human-readable XML tree.
    // copyXmlToBinary() encodes the XML as a compact binary blob.
    if (auto xml = apvts.copyState().createXml())
    {
        copyXmlToBinary(*xml, destData);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// setStateInformation()
//
// Called by the host when loading a project (or preset). Decodes the binary
// blob back to XML, parses it into a ValueTree, and replaces the APVTS state.
// APVTS then notifies all registered listeners (including the UI sliders)
// that parameter values have changed.
// ─────────────────────────────────────────────────────────────────────────────
void TariniAudioProcessor::setStateInformation(
    const void* data,
    int sizeInBytes)
{
    // getXmlFromBinary() decodes the binary blob into an XmlElement.
    // fromXml() parses it back into a ValueTree.
    // replaceState() atomically replaces the APVTS state and notifies listeners.
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
    {
        apvts.replaceState(
            juce::ValueTree::fromXml(*xml));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// createPluginFilter()
//
// The JUCE plugin entry point. The host calls this C-linkage function to
// instantiate the plugin. JUCE_CALLTYPE ensures the correct calling convention
// on all platforms. Returns a heap-allocated processor — the host owns it.
// ─────────────────────────────────────────────────────────────────────────────
juce::AudioProcessor* JUCE_CALLTYPE
createPluginFilter()
{
    return new TariniAudioProcessor();
}