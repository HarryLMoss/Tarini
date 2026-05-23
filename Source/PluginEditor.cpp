/*
===============================================================================

    PluginEditor.cpp
    Author: Harry Moss
    Created: 21st May 2026

    GUI editor implementation for Tarini plugin.

===============================================================================
*/

#include "PluginEditor.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
//
// Sets up the UI. The order here matters:
//   1. Configure slider appearance
//   2. Create attachments (must happen after sliders exist, before addAndMakeVisible)
//   3. Add sliders to the component tree
//   4. Set the window size
//
// The AudioProcessorEditor base class constructor is called with &p so that
// JUCE can associate this editor with its processor.
// ─────────────────────────────────────────────────────────────────────────────
TariniEditor::TariniEditor(TariniAudioProcessor& p)
    : AudioProcessorEditor(&p),
      proc(p)
{
    // ── Tonic slider ──────────────────────────────────────────────────────

    // Rotary style = circular knob, not a linear slider.
    tonicSlider.setSliderStyle(juce::Slider::Rotary);

    // TextBoxBelow: show the numeric value in a box beneath the knob.
    // false = not read-only (user can type a value).
    // 80, 20 = text box width and height in pixels.
    tonicSlider.setTextBoxStyle(
        juce::Slider::TextBoxBelow,
        false,
        80,
        20);

    // ── Gain slider ───────────────────────────────────────────────────────

    gainSlider.setSliderStyle(juce::Slider::Rotary);
    gainSlider.setTextBoxStyle(
        juce::Slider::TextBoxBelow,
        false,
        80,
        20);

    // ── SliderAttachments ─────────────────────────────────────────────────
    //
    // Attachments must be created AFTER the sliders exist and BEFORE the
    // sliders are added to the component tree (addAndMakeVisible).
    //
    // On construction, each SliderAttachment:
    //   - Reads the current parameter value from APVTS and sets the slider
    //   - Sets the slider's range to match the parameter's NormalisableRange
    //   - Installs a listener on the slider: user gesture → atomic write to APVTS
    //   - Installs a listener on the parameter: DAW automation → updates slider
    //
    // The parameter ID string ("tonic", "gain") must exactly match the IDs
    // used in TariniAudioProcessor::makeLayout().
    tonicAttach = std::make_unique<Attach>(
        proc.apvts,    // the APVTS that owns the parameter
        "tonic",       // parameter ID — must match makeLayout()
        tonicSlider);  // the slider to bind

    gainAttach = std::make_unique<Attach>(
        proc.apvts,
        "gain",
        gainSlider);

    // ── Add to component tree ─────────────────────────────────────────────
    //
    // addAndMakeVisible registers the slider as a child of this component
    // and makes it visible. Child components are painted, receive mouse
    // events, and are included in the layout managed by resized().
    addAndMakeVisible(tonicSlider);
    addAndMakeVisible(gainSlider);

    // Set the plugin window size in pixels.
    // resized() is called automatically after setSize().
    setSize(400, 300);
}

// ─────────────────────────────────────────────────────────────────────────────
// paint()
//
// Draws the background and any custom 2D graphics.
// Called by JUCE's rendering engine whenever the component needs repainting
// (on resize, repaint() calls, or host-driven redraws).
//
// All drawing goes through the Graphics context g — never draw to the screen
// directly. JUCE handles double-buffering internally.
// ─────────────────────────────────────────────────────────────────────────────
void TariniEditor::paint(juce::Graphics& g)
{
    // Fill the entire component area with black background.
    g.fillAll(juce::Colours::black);

    // Set the colour for subsequent drawing operations.
    g.setColour(juce::Colours::white);

    // Set font size for the title text.
    g.setFont(20.0f);

    // Draw "Tarini" centred horizontally at the top of the window.
    // getLocalBounds() returns the full component rectangle.
    // centredTop = horizontally centred, vertically aligned to the top.
    // The final argument (1) is the maximum number of lines.
    g.drawFittedText(
        "Tarini",
        getLocalBounds(),
        juce::Justification::centredTop,
        1);
}

// ─────────────────────────────────────────────────────────────────────────────
// resized()
//
// Called whenever the component is resized — including the initial layout
// after construction. Set all child component bounds here.
//
// Never call setSize() or setBounds() from paint() — layout and drawing
// are intentionally separated in JUCE.
// ─────────────────────────────────────────────────────────────────────────────
void TariniEditor::resized()
{
    // setBounds(x, y, width, height) positions each slider within the
    // component's coordinate space (top-left = 0,0).
    //
    // Tonic knob: left side of the window, centred vertically.
    tonicSlider.setBounds(40, 80, 140, 140);

    // Gain knob: right side of the window, same vertical position.
    gainSlider.setBounds(220, 80, 140, 140);
}