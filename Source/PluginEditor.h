/*
===============================================================================

    PluginEditor.h
    Author: Harry Moss
    Created: 21st May 2026

    GUI editor for Tarini plugin.

    TariniEditor is the plugin's user interface. It inherits from
    juce::AudioProcessorEditor, which is the base class for all JUCE plugin
    GUIs. The editor lives exclusively on the message (UI) thread — it must
    never access the audio thread's state directly.

    Communication with the processor happens entirely through APVTS
    SliderAttachments. When the user moves a slider, the attachment writes
    the new value atomically into the APVTS parameter. When a DAW automates
    a parameter, the attachment updates the slider position. All thread-safe,
    no manual listener code required.

    The editor holds a reference to the processor (proc) — safe because JUCE
    guarantees the processor outlives the editor.

===============================================================================
*/

#pragma once

// JuceHeader.h brings in all JUCE modules declared in CMakeLists.txt.
#include <JuceHeader.h>
#include "PluginProcessor.h"

class TariniEditor : public juce::AudioProcessorEditor
{
public:
    // Constructor takes a reference to the processor so it can access apvts.
    // Marked explicit to prevent accidental implicit conversions.
    explicit TariniEditor(TariniAudioProcessor&);

    // Destructor is defaulted. SliderAttachments are unique_ptrs and clean up
    // automatically. It is important that attachments are destroyed BEFORE the
    // sliders they reference — unique_ptr destruction order (reverse of
    // declaration order) handles this correctly here.
    ~TariniEditor() override = default;

    // ── JUCE Component overrides ──────────────────────────────────────────

    // Called by JUCE whenever the component needs to be redrawn.
    // Draw the background and any custom graphics here.
    // Never call repaint() from inside paint() — infinite loop.
    void paint(juce::Graphics&) override;

    // Called whenever the component is resized (including initial layout).
    // Set the bounds of all child components here — never in paint().
    void resized() override;

private:
    // ── Type alias ────────────────────────────────────────────────────────

    // SliderAttachment binds a juce::Slider to an APVTS parameter by ID.
    // It installs a listener on both the Slider and the parameter so that
    // changes in either direction (user gesture or DAW automation) stay
    // synchronised — entirely automatically, with no manual listener code.
    using Attach = juce::AudioProcessorValueTreeState::SliderAttachment;

    // ── Processor reference ───────────────────────────────────────────────

    // Non-owning reference to the processor. Safe because JUCE guarantees
    // the processor outlives all editors it creates.
    TariniAudioProcessor& proc;

    // ── UI components ─────────────────────────────────────────────────────

    // Rotary dial controlling the tonic frequency (110–440 Hz).
    juce::Slider tonicSlider;

    // Rotary dial controlling the master output gain (0.0–1.0).
    juce::Slider gainSlider;

    // Text labels displayed below the sliders. Currently declared but not
    // yet added to the component tree — placeholder for future UI polish.
    juce::Label tonicLabel;
    juce::Label gainLabel;

    // ── Parameter attachments ─────────────────────────────────────────────

    // unique_ptr because attachments must be created after the sliders exist
    // (in the constructor body, not the initialiser list), and destroyed
    // before the sliders are destroyed. unique_ptr handles both requirements.
    //
    // The attachment is constructed with (apvts, parameterID, slider).
    // It immediately sets the slider's range and value from the parameter,
    // then registers listeners on both sides for ongoing synchronisation.
    std::unique_ptr<Attach> tonicAttach;
    std::unique_ptr<Attach> gainAttach;

    // Prevents copy/assignment and registers with JUCE's leak detector.
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TariniEditor)
};