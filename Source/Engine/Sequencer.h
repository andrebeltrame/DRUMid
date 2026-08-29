#pragma once

#include "Types.h"

#include <juce_audio_basics/juce_audio_basics.h>

#include <atomic>
#include <random>
#include <vector>

namespace drumid
{

/** Transport-locked step player.

    Works entirely in PPQ (quarter notes since the start of the timeline) rather
    than counting samples, so looping, tempo changes and dragging the playhead
    around in Ableton all land on the right step. Events are emitted with
    sample-accurate offsets inside the block.
*/
class Sequencer
{
public:
    void prepare (double sampleRate);
    void reset();

    /** Emit this block's notes.
        @param ppqStart  playhead position in quarter notes at the block start
        @param bpm       host tempo
        @param playing   host transport state */
    void process (const Kit& kit,
                  juce::MidiBuffer& midi,
                  int numSamples,
                  double ppqStart,
                  double bpm,
                  bool playing);

    /** Free-running clock for the standalone build, where there is no host. */
    void processFreeRunning (const Kit& kit, juce::MidiBuffer& midi, int numSamples, double bpm);

    /** 0-based step under the playhead, for the UI. -1 when stopped. */
    int currentStep() const noexcept { return playStep.load(); }

private:
    struct PendingOff
    {
        int note;
        int samplesRemaining;
    };

    void renderWindow (const Kit& kit, juce::MidiBuffer& midi, int numSamples,
                       double ppqStart, double ppqPerSample);
    void emitNote (juce::MidiBuffer& midi, int sampleOffset, int note, float velocity, int gateSamples);
    void flushPendingOffs (juce::MidiBuffer& midi, int numSamples);
    void allNotesOff (juce::MidiBuffer& midi, int sampleOffset);

    double sr        = 44100.0;
    double freePpq   = 0.0;
    bool   wasPlaying = false;

    std::vector<PendingOff> pending;
    std::atomic<int> playStep { -1 };
    std::mt19937 rng { 20260829u };
    std::uniform_real_distribution<float> dist { 0.0f, 1.0f };
};

} // namespace drumid
