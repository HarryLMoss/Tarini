# Tarini

> *"She who carries across."* — From the Sanskrit root *tara*, to transcend.

Tarini is a real-time generative drone engine built in **C++ with the JUCE framework**, implementing DSP fundamentals drawn from Indian classical music theory — specifically the tuning systems, harmonic structures, and resonance principles of the Tanpura and Shruti drone traditions.

This is **Stage 1 of a larger embedded and desktop audio architecture**, intentionally designed from the ground up with real-time audio constraints, clean DSP implementation, and embedded-style thinking. The project bridges the acoustic principles documented in the *Sangita Ratnakara* and *Natya Shastra* with modern audio engineering practice.

---

## Overview

Tarini generates a continuous, harmonically rich drone based on Tanpura tuning — four voices tuned to SA, PA, SA (upper octave), and a detuned SA for organic chorus. The system uses additive synthesis, LFO-based pitch modulation, bandpass filtering, and stereo imaging to simulate the resonant body of a plucked string instrument in real time.

The architecture is designed to be portable, efficient, and scalable — qualities directly applicable to embedded audio systems running on ARM Cortex platforms with tight latency and memory budgets.

---

## DSP Implementation

The core audio engine (`DroneVoice`) implements:

- **Additive synthesis** — fundamental plus harmonic partials (2nd and 3rd harmonics) modelling the overtone structure of a plucked string
- **Sample-rate-independent LFO modulation** — subtle sinusoidal drift (~0.1 Hz) simulating natural string intonation variation, correctly derived from `sampleRate` at `prepareToPlay` time
- **State Variable TPT Filter** (`juce::dsp::StateVariableTPTFilter`) — bandpass filtered at 900 Hz for body resonance simulation. TPT topology chosen for numerical stability at embedded-target sample rates
- **Stereo imaging** — per-voice left/right gain matrix to simulate the physical spread of a multi-string instrument

All DSP runs inside the JUCE `processBlock` callback, respecting real-time audio thread constraints — no heap allocation, no blocking calls, no locks in the hot path. All resources are pre-allocated in `prepareToPlay`.

### Parameter management

Parameters are managed via `juce::AudioProcessorValueTreeState` (APVTS), providing:
- Thread-safe atomic parameter reads from the audio thread
- Automatic DAW state save/recall (`getStateInformation` / `setStateInformation`)
- UI binding via `SliderAttachment` — no manual listener boilerplate

### Tuning system

```
Voice 1:  SA  — tonic (fundamental)
Voice 2:  PA  — perfect fifth (tonic × 1.5)
Voice 3:  SA' — upper octave (tonic × 2.0)
Voice 4:  SA' — upper octave + 1% detune (tonic × 2.0 × 1.01)
```

This mirrors the authentic Tanpura tuning system, producing the characteristic *jiva* (shimmer) through interference patterns between the detuned upper voices.

---

## Architecture

```
Tarini/
│
├── CMakeLists.txt                 ← CMake build configuration (Standalone + VST3)
├── CMakeUserPresets.json          ← Local build preset (Ninja + MSVC, not committed)
├── README.md
├── LICENSE
│
├── Source/
│   ├── PluginProcessor.h/.cpp     ← AudioProcessor: DSP, APVTS, plugin lifecycle
│   ├── PluginEditor.h/.cpp        ← AudioProcessorEditor: UI, SliderAttachments
│   └── DroneVoice.h/.cpp          ← Isolated DSP voice: additive synth + filter + LFO
│
└── Theory Files/
    ├── Doctrine of Shruti.txt
    ├── Raga and Timings.xlsx
    ├── Research Links
    ├── Shruti-Nidarshanam-Sarana Chatushtaya.pdf
    ├── Shruti-Veena-Swara-Sthapana.pdf
    └── The-Doctrine-of-Shruti-in-Indian-Music.pdf
```

The plugin follows the standard JUCE AudioProcessor / AudioProcessorEditor split:

- **`TariniAudioProcessor`** — owns all DSP state, parameters (APVTS), and plugin lifecycle. Lives on both the audio thread (`processBlock`) and the message thread (state management).
- **`TariniEditor`** — owns all UI components. Lives exclusively on the message thread. Communicates with the processor only via APVTS `SliderAttachment` — never touches DSP state directly.
- **`DroneVoice`** — a self-contained mono DSP voice. Stateless beyond its own phase accumulators. Intentionally isolated for portability to future embedded or MATLAB targets.

The `Theory Files/` directory contains the primary source material underpinning Tarini's musical architecture — classical Sanskrit treatises and sruti research that inform the tuning systems, harmonic decisions, and therapeutic application layer. This is not background reading; it is the design specification.

---

## Technologies

| Technology | Usage |
|---|---|
| C++17 | Core audio engine and application logic |
| JUCE Framework | Real-time audio I/O, DSP primitives, GUI, plugin infrastructure |
| CMake 3.22+ | Build system — Standalone and VST3 targets |
| Ninja | Build backend (fast, generator-agnostic) |
| MSVC (VS 2022+) | Windows compiler |
| Git / GitHub | Version control |
| Python | Data analysis, musical principle extraction from source texts |

---

## Build & Run

### Requirements

- [JUCE](https://github.com/juce-framework/JUCE) cloned locally (see below)
- CMake 3.22+
- Ninja (`winget install Ninja-build.Ninja` on Windows)
- MSVC — Visual Studio 2022 or Visual Studio Community 2026 with C++ workload

### Setup

```bash
# Clone JUCE once alongside your projects
git clone --depth 1 https://github.com/juce-framework/JUCE.git ../JUCE

# Clone Tarini
git clone https://github.com/HarryLMoss/Tarini.git
cd Tarini
```

### Configure

Create `CMakeUserPresets.json` in the repo root (this file is gitignored — each developer has their own):

```json
{
    "version": 3,
    "configurePresets": [
        {
            "name": "default",
            "displayName": "Default (Ninja + MSVC)",
            "generator": "Ninja",
            "binaryDir": "${sourceDir}/build",
            "cacheVariables": {
                "CMAKE_BUILD_TYPE": "Debug",
                "CMAKE_C_COMPILER": "cl",
                "CMAKE_CXX_COMPILER": "cl",
                "JUCE_DIR": "../JUCE"
            }
        }
    ]
}
```

Adjust `JUCE_DIR` to wherever you cloned JUCE.

### Build

Open a **Developer PowerShell for Visual Studio** (required so `cl.exe` is on PATH), then:

```powershell
cmake --preset default
cmake --build build
```

First build takes 5–10 minutes (JUCE compiles all modules). Subsequent builds are seconds.

### Run

**Standalone app:**
```powershell
.\build\Tarini_artefacts\Debug\Standalone\Tarini.exe
```

**VST3** (copy to your system VST3 folder for DAW use):
```powershell
xcopy /E /I /Y "build\Tarini_artefacts\Debug\VST3\Tarini.vst3" "%CommonProgramFiles%\VST3\Tarini.vst3"
```

---

## Roadmap

### Stage 1 — Real-time DSP foundation *(current)*
- [x] Additive synthesis drone engine in JUCE C++
- [x] Sample-rate-independent LFO modulation and bandpass body resonance
- [x] Stereo imaging across four string voices
- [x] Migrated from legacy Projucer to modern CMake build system
- [x] Migrated from AudioAppComponent to AudioProcessor + APVTS plugin architecture
- [x] Standalone and VST3 build targets
- [x] Thread-safe parameter management via AudioProcessorValueTreeState
- [ ] GoogleTest unit test suite
- [ ] GitHub Actions CI
- [ ] Preset save/load with JSON format

### Stage 2 — Musical intelligence layer
- Raga-specific tuning systems (just intonation, sruti variants per raga)
- MIDI input and real-time tonic tracking
- Harmonic analysis and adaptive drone modulation
- Microtonal intonation and raga time-of-day scheduling

### Stage 3 — Embedded and plugin targets
- DSP core ported to bare-metal Embedded C for ARM Cortex-M4/M7
- Fixed-point arithmetic optimisation for microcontroller deployment
- AU plugin build (macOS)
- SIMD optimisation, lock-free parameter queues, USB/SPI/I2C peripheral integration
- Standalone hardware instrument — no laptop, stage and studio ready

### Stage 4 — Intelligent generative system
- Deep learning fusion of Indian classical principles with contemporary genres
- Raga recommendation and mood-adaptive drone modulation
- Therapeutic application grounded in the Natya Shastra Rasa framework
- DAW integration via MIDI output

### Stage 5 — Live performance and mass-meditation deployment
- Real-time performer tracking — pitch detection from live instrument or voice
- Audience-responsive harmonic and rhythmic modulation
- Spatial and ambisonics rendering for large venue PA systems
- DMX / OSC bridge for synchronised lighting and visual output
- Mass-meditation mode — autonomous, time-of-day aware, Rasa-guided generative state

---

## Relevance to Embedded Audio Engineering

The Tarini DSP core is explicitly designed with embedded constraints in mind:

- **No dynamic allocation in the audio path** — all voices pre-allocated and prepared at `prepareToPlay`, never in `processBlock`
- **Isolated DSP voice architecture** — `DroneVoice` is intentionally self-contained for future embedded deployment
- **TPT filter design** — Topology-Preserving Transform chosen for numerical stability at low sample rates directly relevant to embedded targets running at 48 kHz or below
- **Phase accumulator design** — efficient, portable, no lookup tables required
- **Sample-rate-independent modulation** — LFO increment computed once at prepare time, correct across all sample rates

Future stages will target ARM Cortex-M4/M7 platforms, implementing the same algorithms under fixed-point arithmetic with profiled cycle budgets.

---

## Relevance to Audio Plugin Development

The JUCE-based implementation demonstrates:

- Real-time audio callback design (`processBlock`) with correct thread discipline
- Full `AudioProcessor` / `AudioProcessorEditor` architectural separation
- `AudioProcessorValueTreeState` (APVTS) — thread-safe parameter management, DAW automation, state serialisation
- `juce::dsp::ProcessSpec` initialisation and `prepare()` / `releaseResources()` lifecycle
- `juce::dsp::StateVariableTPTFilter` — professional DSP primitive usage
- `SliderAttachment` binding — UI to parameter without manual listeners
- CMake-based JUCE build: `juce_add_plugin`, `juce_generate_juce_header`, multi-format targets
- Standalone and VST3 format builds from a single codebase

---

## Musical Foundation

Tarini draws on two foundational texts of Indian classical music theory:

**Sangita Ratnakara** (Sarangadeva, 13th century) — the definitive treatise on Indian music, covering Nada (primordial sound), Sruti (microtonal intervals), Raga classification, and performance theory.

**Natya Shastra** (Bharata Muni) — the ancient Sanskrit text governing performing arts, including the relationship between musical modes, emotional states (Rasa), and their physiological and psychological effects on listeners.

These principles inform the tuning systems, harmonic choices, and therapeutic application layer of the Tarini architecture — not as aesthetic decoration, but as a genuine theoretical framework for generative audio design.

---

## Contributing

Contributions welcome. Please follow this workflow:

```bash
git checkout -b feature/your-feature
git commit -m 'Add feature'
git push origin feature/your-feature
# Open a Pull Request
```

---

## License

GNU General Public License v3.0 — see `LICENSE` for details.

---

## Acknowledgements

- **Sarangadeva** — Sangita Ratnakara
- **Bharata Muni** — Natya Shastra
- **Narada** — Naradiya Shiksha
- **The JUCE Framework Team** — real-time audio infrastructure
- **The Python and C++ open source communities**

---

## Contact

Enquiries and collaboration: harrymoss33@gmail.com
Other audio and DSP projects: [github.com/HarryLMoss](https://github.com/HarryLMoss)
