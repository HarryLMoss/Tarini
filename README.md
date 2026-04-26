# Tarini

> *"She who carries across."* — From the Sanskrit root *tara*, to transcend.

Tarini is a real-time generative drone engine built in **C++ with the JUCE framework**, implementing DSP fundamentals drawn from Indian classical music theory — specifically the tuning systems, harmonic structures, and resonance principles of the Tanpura and Shruti drone traditions.

This is **Stage 1 of a larger embedded and desktop audio architecture**, intentionally designed from the ground up with real-time audio constraints, clean DSP implementation, and embedded-style thinking. The project bridges the acoustic principles documented in the *Sangita Ratnakara* and *Natya Shastra* with modern audio engineering practice.

---

## Overview

Tarini generates a continuous, harmonically rich drone based on Tanpura tuning — four strings tuned to SA, PA, SA (upper octave), and a detuned SA for organic chorus. The system uses additive synthesis, LFO-based pitch modulation, bandpass filtering, and stereo imaging to simulate the resonant body of a plucked string instrument in real time.

The architecture is designed to be portable, efficient, and scalable — qualities directly applicable to embedded audio systems running on ARM Cortex platforms with tight latency and memory budgets.

---

## DSP Implementation

The core audio engine (`DroneVoice`) implements:

- **Additive synthesis** — fundamental plus harmonic partials (2nd and 3rd harmonics) modelling the overtone structure of a plucked string
- **LFO pitch modulation** — subtle sinusoidal drift simulating natural string intonation variation
- **State Variable TPT Filter** (`juce::dsp::StateVariableTPTFilter`) — bandpass filtered at 900Hz for body resonance simulation
- **Stereo imaging** — per-string left/right gain matrix to simulate the physical spread of a multi-string instrument
- **Real-time parameter smoothing** — gain and frequency updates handled safely within the audio callback

All DSP runs inside the JUCE `getNextAudioBlock` callback, respecting real-time audio thread constraints — no heap allocation, no blocking calls, no locks in the hot path.

### Tuning system

```
String 1:  SA  — tonic (fundamental)
String 2:  PA  — perfect fifth (tonic × 1.5)
String 3:  SA' — upper octave (tonic × 2.0)
String 4:  SA' — upper octave + 1% detune (tonic × 2.0 × 1.01)
```

This mirrors the authentic Tanpura tuning system, which produces the characteristic *jiva* (shimmer) through interference patterns between the detuned upper strings.

---

## Architecture

```
tarini/
│
├── README.md
├── LICENSE
│
├── Theory/
│   ├── Doctrine of Shruti.txt
│   ├── Raga and Timings.xlsx
│   ├── Research Links
│   ├── Shruti-Nidarshanam-Sarana Chatushtaya.pdf
│   ├── Shruti-Veena-Swara-Sthapana.pdf
│   └── The-Doctrine-of-Shruti-in-Indian-Music.pdf
│
└── Source/                        ← Stage 1 JUCE implementation (in progress)
├── Main.cpp
├── MainComponent.h/.cpp
├── DroneVoice.h/.cpp
└── Tarini.jucer
```

The `Theory/` directory contains the primary source material underpinning Tarini's musical architecture — classical Sanskrit treatises and sruti research that inform the tuning systems, harmonic decisions, and therapeutic application layer. This is not background reading; it is the design specification.

The `DroneVoice` DSP class is intentionally isolated — stateless beyond its own phase accumulators — designed from the outset for portability to bare-metal Embedded C, MATLAB prototyping, or a VST plugin target.

---

## Technologies

| Technology | Usage |
|---|---|
| C++ (modern) | Core audio engine and application logic |
| JUCE Framework | Real-time audio I/O, DSP primitives, GUI |
| Python | Data analysis, musical principle extraction from source texts |
| GPT-4 | Automated analysis of Sangita Ratnakara and Natya Shastra |
| Docker | Containerisation for consistent build environments |
| CMake | Build system (roadmap) |
| Git | Version control |

---

## Build & Run

### Requirements

- JUCE 7.x or above
- C++17 compatible compiler (MSVC, Clang, GCC)
- Projucer or CMake

### Setup

```bash
git clone https://github.com/HarryLMoss/tarini.git
cd tarini
```

Open `Tarini.jucer` in Projucer, export to your target IDE, and build. Targets: Windows, macOS, Linux.

---

## Roadmap

### Stage 1 — Real-time DSP foundation *(current)*
- Additive synthesis drone engine in JUCE C++
- LFO modulation and bandpass body resonance simulation
- Stereo imaging across four string voices
- Desktop GUI with tonic frequency and master gain control

### Stage 2 — Musical intelligence layer
- Raga-specific tuning systems (just intonation, sruti variants per raga)
- MIDI input and real-time tonic tracking
- Harmonic analysis and adaptive drone modulation
- Microtonal intonation and raga time-of-day scheduling

### Stage 3 — Embedded and plugin targets
- DSP core ported to bare-metal Embedded C for ARM Cortex-M/A
- Fixed-point arithmetic optimisation for microcontroller deployment
- VST3 / AU plugin build via JUCE plugin architecture
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

## Musical Foundation

Tarini draws on two foundational texts of Indian classical music theory:

**Sangita Ratnakara** (Sarangadeva, 13th century) — the definitive treatise on Indian music, covering Nada (primordial sound), Sruti (microtonal intervals), Raga classification, and performance theory.

**Natya Shastra** (Bharata Muni) — the ancient Sanskrit text governing performing arts, including the relationship between musical modes, emotional states (Rasa), and their physiological and psychological effects on listeners.

These principles inform the tuning systems, harmonic choices, and therapeutic application layer of the Tarini architecture — not as aesthetic decoration, but as a genuine theoretical framework for generative audio design.

---

## Relevance to Embedded Audio Engineering

The Tarini DSP core is explicitly designed with embedded constraints in mind:

- **No dynamic allocation in the audio path** — all voices pre-allocated and prepared at `prepareToPlay`
- **Fixed block size processing** — compatible with RTOS audio scheduling
- **Isolated DSP voice architecture** — `DroneVoice` can be ported to bare-metal C with minimal refactoring
- **State variable filter implementation** — TPT (Topology-Preserving Transform) design chosen for its numerical stability at low sample rates, directly relevant to embedded targets running at 48kHz or below
- **Phase accumulator design** — efficient, portable, no lookup tables required

Future stages will target ARM Cortex-M4/M7 platforms, implementing the same algorithms under fixed-point arithmetic with profiled cycle budgets — the core engineering challenge of embedded audio DSP.

---

## Relevance to Audio Plugin Development

The JUCE-based desktop implementation demonstrates:

- Real-time audio callback design (`getNextAudioBlock`)
- `juce::dsp::ProcessSpec` initialisation and `prepare()` lifecycle
- `juce::dsp::StateVariableTPTFilter` — professional DSP primitive usage
- Slider listener pattern and thread-safe parameter updates
- Cross-platform GUI layout (`paint`, `resized`)
- Scalable towards VST3 / AU plugin architecture in Stage 3

---

## Contributing

Contributions welcome. Please follow this workflow:

```bash
# Fork the repository
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
- **OpenAI** — GPT models used in musical text analysis

---

## Contact

Enquiries and collaboration: harrymoss33@gmail.com

Other audio and DSP projects: [github.com/HarryLMoss](https://github.com/HarryLMoss)
