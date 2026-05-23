/*
===============================================================================

    PluginProcessor.h
    Author: Harry Moss
    Created: 21st May 2026

    Main audio processor for Tarini.

    TariniAudioProcessor is the heart of the plugin. It inherits from
    juce::AudioProcessor, which is the base class for all JUCE plugins
    and standalone audio apps built with the plugin architecture.

    Responsibilities:
      - Owns all DSP objects (four DroneVoice instances)
      - Manages plugin parameters via AudioProcessorValueTreeState (APVTS)
      - Implements the plugin lifecycle: prepareToPlay, processBlock,
        releaseResources
      - Handles DAW state save/recall via getStateInformation /
        setStateInformation

    Threading model:
      - processBlock() runs on the realtime audio thread
      - All other methods run on the DAW's message/UI thread
      - APVTS parameter values are stored as std::atomic<float> internally,
        making them safe to read from the audio thread without locks

===============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DroneVoice.h"

class TariniAudioProcessor : public juce::AudioProcessor
{
public:
    TariniAudioProcessor();

    // Default destructor is sufficient — all members clean up automatically.
    ~TariniAudioProcessor() override = default;

    // ── Plugin lifecycle ──────────────────────────────────────────────────

    // Called by the host before audio starts. Use this to allocate buffers,
    // initialise DSP objects, and compute anything that depends on sampleRate
    // or block size. Must NOT be called while processBlock() is running.
    void prepareToPlay(double sampleRate,
                       int samplesPerBlock) override;

    // Called when the host stops audio. Release any resources acquired in
    // prepareToPlay. Empty here because DroneVoice has no external resources.
    void releaseResources() override {}

    // ── Audio processing ──────────────────────────────────────────────────

    // The hot path. Called by the host on the realtime audio thread every
    // ~5-12ms (depending on buffer size / sample rate). Must return before
    // the next callback fires — any delay causes an audible dropout.
    //
    // Realtime constraints: no memory allocation, no mutex locks, no file I/O,
    // no system calls. Read parameters via atomic load only.
    void processBlock(juce::AudioBuffer<float>&,
                      juce::MidiBuffer&) override;

    // ── Editor (UI) ───────────────────────────────────────────────────────

    // Instantiates the GUI. Called on the message thread when the DAW opens
    // the plugin window. Returns a heap-allocated TariniEditor; the host owns
    // the lifetime and deletes it when the window is closed.
    juce::AudioProcessorEditor* createEditor() override;

    // Tells the host this plugin has a custom UI.
    bool hasEditor() const override { return true; }

    // ── Plugin metadata ───────────────────────────────────────────────────

    // Name shown in the DAW's plugin list.
    const juce::String getName() const override { return "Tarini"; }

    // Tarini is a drone generator — it does not accept or produce MIDI.
    bool acceptsMidi()  const override { return false; }
    bool producesMidi() const override { return false; }

    // No reverb tail — output goes to zero immediately when processing stops.
    double getTailLengthSeconds() const override { return 0.0; }

    // ── Program (preset) interface ────────────────────────────────────────
    // JUCE requires these to be implemented. We don't use the program system
    // (we use APVTS state for presets instead), so we return minimal values.
    int  getNumPrograms()                                   override { return 1; }
    int  getCurrentProgram()                                override { return 0; }
    void setCurrentProgram(int)                             override {}
    const juce::String getProgramName(int)                  override { return {}; }
    void changeProgramName(int, const juce::String&)        override {}

    // ── State persistence (DAW save/recall) ───────────────────────────────

    // Called by the host when saving a project. Serialises the APVTS state
    // (all parameter values) to a binary blob stored in the DAW session file.
    void getStateInformation(juce::MemoryBlock&) override;

    // Called by the host when loading a project. Deserialises the binary blob
    // and restores all parameter values via APVTS.
    void setStateInformation(const void*, int) override;

    // ── Parameters ────────────────────────────────────────────────────────

    // AudioProcessorValueTreeState: JUCE's modern parameter management system.
    // Declared public so TariniEditor can attach sliders to it directly via
    // SliderAttachment — the standard JUCE pattern for thread-safe UI binding.
    //
    // Internally, APVTS stores each parameter as a std::atomic<float>, which
    // means the audio thread can safely call getRawParameterValue()->load()
    // in processBlock() without any locks.
    juce::AudioProcessorValueTreeState apvts;

private:
    // ── Private helpers ───────────────────────────────────────────────────

    // Builds and returns the parameter layout used to construct APVTS.
    // Declared static because it is called from the constructor's member
    // initialiser list before the object is fully constructed.
    static juce::AudioProcessorValueTreeState::ParameterLayout makeLayout();

    // Reads the current tonic value from APVTS and updates all four voice
    // frequencies and gains. Called from prepareToPlay and at the start of
    // every processBlock to pick up parameter changes from the UI.
    void updateVoices();

    // ── DSP objects ───────────────────────────────────────────────────────

    // Four DroneVoice instances — one per Tanpura string:
    //   voices[0]: SA  — tonic fundamental
    //   voices[1]: PA  — perfect fifth (tonic × 1.5)
    //   voices[2]: SA' — upper octave (tonic × 2.0)
    //   voices[3]: SA' — upper octave, 1% sharp detune (tonic × 2.02)
    //
    // Stack-allocated (not heap): all four voices exist for the full lifetime
    // of the processor. No dynamic allocation ever happens in the audio path.
    DroneVoice voices[4];

    // Prevents accidental copy/assignment of the processor, and registers it
    // with JUCE's leak detector.
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TariniAudioProcessor)
};