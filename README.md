# Drummy

MIDI drum pattern generator for **melodic house**, **organic house** and **techno**,
built for Ableton Live and Drum Racks.

Pick your lanes, hit GENERATE, drag the MIDI into a clip. No sound of its own —
Drummy drives your rack.

---

## Status

Phase 1 (vertical slice) — plugin loads, generates, plays in sync and exports.

| | |
|---|---|
| Formats | VST3, Standalone (universal binary: arm64 + x86_64) |
| Lanes | Kick, Clap, Closed Hat, Open Hat, Shaker |
| Genres | Melodic House, Organic House, Techno |
| Seed bank | 50 curated patterns |
| Pattern length | 1, 2 or 4 bars |

AU is wired up in `CMakeLists.txt` but off by default because it needs a full
Xcode install (Command Line Tools alone won't build it). With Xcode present:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DDRUMMY_BUILD_AU=ON
```

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j8
```

JUCE 8.0.9 is pulled in automatically by CMake. The VST3 is copied to
`~/Library/Audio/Plug-Ins/VST3/` on every build.

## Tests

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DDRUMMY_BUILD_TESTS=ON
cmake --build build --target DrummyTests -j8
./build/DrummyTests_artefacts/Release/DrummyTests
```

Prints the generated grids for all three genres and asserts the things that are
hard to hear a bug in: the kick lands on the quarters, techno's hat stays off
the downbeat, bar 2 isn't a copy of bar 1, swing is real micro-timing, a locked
lane survives GENERATE, a seed is reproducible, and the exported .mid contains
every hit on the right pad at exactly the right clip length.

## Using it in Ableton

**To commit a pattern to a clip (the main workflow):** drag `DRAG KIT MIDI`
straight into a clip slot. Every enabled lane comes across on its mapped note,
with swing and humanize baked into the timing — what you drag is what you heard.

**To audition live:** Drummy is an instrument that outputs MIDI, so it goes on
its own MIDI track. On the Drum Rack track set `MIDI From` → the Drummy track →
`Drummy`, and `Monitor: In`.

## Note mapping

The pattern is only useful if it fires the right pad:

- **GM / Drum Rack** — 36 kick, 39 clap, 42 closed hat, 46 open hat, 70 shaker.
  Matches Ableton's factory racks.
- **Drum Rack 4x4** — C1 upward, chromatic, one pad per lane. For racks laid out
  by pad position.
- **Custom** — drag any lane's note readout up or down.

## The pattern brain

Three layers, because a pure random generator sounds random and a pure library
gets repetitive:

1. **Curated seeds** (`Engine/PatternLibrary.cpp`) — genre-specific patterns with
   velocity and articulation already in them. Energy gates which seeds are even
   eligible, so turning it up reaches for busier material instead of sprinkling
   random hits on a sparse pattern.
2. **Compatibility rules** (`Engine/Generator.cpp`) — lanes generate in dependency
   order and react to each other. Open hats move off the kick instead of getting
   masked by it; hats duck under the kick and the clap; percussion is displaced
   into the gaps between kicks, hard in organic house, not at all in melodic.
   In techno the closed hat is kept off the downbeat, because the offbeat-only
   placement *is* the genre.
3. **Variation** — swing as real micro-timing, humanize, probability on ghost
   notes, 32nd ratchets, bar-to-bar mutation so 2 and 4 bar patterns don't read
   as a 1-bar loop, and end-of-phrase fills.

Everything is driven by the seed number, so the same seed rebuilds the same kit.
That is what makes **lock + reroll** work: keep the kick you liked, hit GENERATE,
and only the unlocked lanes change.

### What each genre actually knows

- **Melodic house** — open hat on the 8th-note offbeats, 16th hats with the
  offbeat accent, light swing, ghost pickups into the bar.
- **Organic house** — hand percussion carries it: tresillo, son clave 3-2, rumba
  clave 3-2, E(5,16) and E(7,16). Clap often only on 4. Heavier swing and much
  wider velocity spread, which is what makes it sound played rather than
  programmed.
- **Techno** — straight, zero swing, closed hat on the offbeat, sparse
  syncopated metal placed between the kicks, 32nd ratchet rolls into the phrase.

## Editing the grid

| | |
|---|---|
| click / drag | paint steps on and off |
| alt + drag | velocity |
| double click | cycle ratchet (1 → 2 → 3) |
| lane name | enable / disable the lane |
| `L` | lock — GENERATE will not touch this lane |
| `M` | mute |
| `R` | reroll just this lane |
| drag the note | retune the lane (switches the map to Custom) |

Step blocks are drawn shifted by their real micro-timing, so swing and humanize
are visible and not just audible.

## Layout

```
Source/
  PluginProcessor.*     kit state, double-buffered to the audio thread
  PluginEditor.*        UI, MIDI drag-out tiles
  Engine/
    Types.h             Step / LanePattern / Kit / GenSettings
    PatternLibrary.*    the curated seed bank
    Generator.*         the three-layer brain + Euclidean rhythms
    NoteMap.*           GM / 4x4 / Custom lane mapping
    Sequencer.*         PPQ-locked, sample-accurate playback
    MidiExport.*        kit or single lane to .mid
  UI/
    StepGrid.*          lane headers + step grid
    DrummyLookAndFeel.*
```

## Not there yet

- More lanes: snare, ghost snare, rim, conga, bongo, tom, ride, crash, tambourine, FX
- Host-automatable parameters (state saves and recalls, but nothing is automatable yet)
- Preset browser
- Pattern-length-per-lane (polymeter)
- AU build, pending a full Xcode install

## Licence

Uses [JUCE](https://juce.com) 8, which is GPL3 for open source and requires a
paid licence for closed-source distribution. Licence for Drummy itself: TBD.
