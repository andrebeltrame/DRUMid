#include "Sequencer.h"

#include <cmath>

namespace drummy
{

static constexpr double kPpqPerStep = 0.25;   // one 16th note
static constexpr double kGateMs     = 25.0;   // drum racks only need a trigger

void Sequencer::prepare (double sampleRate)
{
    sr = sampleRate > 0.0 ? sampleRate : 44100.0;
    pending.reserve (64);
    reset();
}

void Sequencer::reset()
{
    pending.clear();
    freePpq = 0.0;
    wasPlaying = false;
    playStep.store (-1);
}

void Sequencer::emitNote (juce::MidiBuffer& midi, int sampleOffset, int note, float velocity, int gateSamples)
{
    const auto v = (juce::uint8) juce::jlimit (1, 127, (int) std::lround (velocity * 127.0f));

    midi.addEvent (juce::MidiMessage::noteOn (1, note, v), sampleOffset);
    pending.push_back ({ note, sampleOffset + gateSamples });
}

void Sequencer::flushPendingOffs (juce::MidiBuffer& midi, int numSamples)
{
    for (int i = (int) pending.size(); --i >= 0;)
    {
        auto& p = pending[(size_t) i];

        if (p.samplesRemaining < numSamples)
        {
            midi.addEvent (juce::MidiMessage::noteOff (1, p.note),
                           juce::jmax (0, p.samplesRemaining));
            pending.erase (pending.begin() + i);
        }
        else
        {
            p.samplesRemaining -= numSamples;
        }
    }
}

void Sequencer::allNotesOff (juce::MidiBuffer& midi, int sampleOffset)
{
    for (auto& p : pending)
        midi.addEvent (juce::MidiMessage::noteOff (1, p.note), sampleOffset);

    pending.clear();
}

void Sequencer::renderWindow (const Kit& kit, juce::MidiBuffer& midi, int numSamples,
                              double ppqStart, double ppqPerSample)
{
    const int    numSteps  = juce::jlimit (1, kMaxSteps, kit.numSteps);
    const double patLenPpq = numSteps * kPpqPerStep;
    const double ppqEnd    = ppqStart + numSamples * ppqPerSample;
    const int    gate      = juce::jmax (1, (int) (kGateMs * 0.001 * sr));

    // Which pattern repetition are we inside? Scan the neighbours too so a note
    // nudged early by swing or humanize at the loop seam still fires.
    const double cycle = std::floor (ppqStart / patLenPpq);

    for (int c = -1; c <= 1; ++c)
    {
        const double base = (cycle + c) * patLenPpq;

        for (int laneIdx = 0; laneIdx < kNumLanes; ++laneIdx)
        {
            const auto& ls = kit.lanes[(size_t) laneIdx];

            if (! ls.enabled)
                continue;

            const auto& pat = kit.patterns[(size_t) laneIdx];

            for (int i = 0; i < numSteps; ++i)
            {
                const auto& st = pat.steps[(size_t) i];

                if (! st.on)
                    continue;

                const int   ratchet  = juce::jlimit (1, 4, st.ratchet);
                const double stepPpq = base + i * kPpqPerStep + st.micro * kPpqPerStep;

                for (int r = 0; r < ratchet; ++r)
                {
                    const double t = stepPpq + (kPpqPerStep / ratchet) * r;

                    if (t < ppqStart || t >= ppqEnd)
                        continue;

                    if (st.probability < 1.0f && dist (rng) > st.probability)
                        continue;

                    // Rolls taper so they read as one gesture, not four hits.
                    const float ratchetScale = (r == 0) ? 1.0f : 0.72f - 0.08f * r;
                    const float vel = juce::jlimit (0.02f, 1.0f, st.velocity * ls.gain * ratchetScale);

                    const int offset = juce::jlimit (0, numSamples - 1,
                                                     (int) std::lround ((t - ppqStart) / ppqPerSample));

                    emitNote (midi, offset, ls.midiNote, vel, gate);
                }
            }
        }
    }

    const int step = (int) std::floor (std::fmod (std::fmod (ppqStart, patLenPpq) + patLenPpq, patLenPpq)
                                       / kPpqPerStep);
    playStep.store (juce::jlimit (0, numSteps - 1, step));
}

void Sequencer::process (const Kit& kit, juce::MidiBuffer& midi, int numSamples,
                         double ppqStart, double bpm, bool playing)
{
    if (numSamples <= 0)
        return;

    if (! playing)
    {
        if (wasPlaying)
            allNotesOff (midi, 0);

        wasPlaying = false;
        playStep.store (-1);
        flushPendingOffs (midi, numSamples);
        return;
    }

    wasPlaying = true;

    const double safeBpm = (bpm > 1.0 && bpm < 999.0) ? bpm : 120.0;
    const double ppqPerSample = safeBpm / 60.0 / sr;

    flushPendingOffs (midi, numSamples);
    renderWindow (kit, midi, numSamples, ppqStart, ppqPerSample);
}

void Sequencer::processFreeRunning (const Kit& kit, juce::MidiBuffer& midi, int numSamples, double bpm)
{
    if (numSamples <= 0)
        return;

    wasPlaying = true;

    const double safeBpm = (bpm > 1.0 && bpm < 999.0) ? bpm : 120.0;
    const double ppqPerSample = safeBpm / 60.0 / sr;

    flushPendingOffs (midi, numSamples);
    renderWindow (kit, midi, numSamples, freePpq, ppqPerSample);

    freePpq += numSamples * ppqPerSample;
}

} // namespace drummy
