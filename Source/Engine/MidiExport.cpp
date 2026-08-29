#include "MidiExport.h"

namespace drumid
{

static constexpr int    kTicksPerQuarter = 960;
static constexpr double kTicksPerStep    = kTicksPerQuarter * 0.25;   // one 16th

juce::MidiMessageSequence MidiExport::toSequence (const Kit& kit, int laneFilter)
{
    juce::MidiMessageSequence seq;

    const int numSteps = juce::jlimit (1, kMaxSteps, kit.numSteps);
    const int gate     = (int) (kTicksPerStep * 0.5);

    for (int laneIdx = 0; laneIdx < kNumLanes; ++laneIdx)
    {
        if (laneFilter >= 0 && laneFilter != laneIdx)
            continue;

        const auto& ls = kit.lanes[(size_t) laneIdx];

        if (! ls.enabled)
            continue;

        const auto& pat = kit.patterns[(size_t) laneIdx];

        for (int i = 0; i < numSteps; ++i)
        {
            const auto& st = pat.steps[(size_t) i];

            if (! st.on)
                continue;

            const int ratchet = juce::jlimit (1, 4, st.ratchet);

            for (int r = 0; r < ratchet; ++r)
            {
                const double tick = (i + st.micro) * kTicksPerStep
                                  + (kTicksPerStep / ratchet) * r;

                const float ratchetScale = (r == 0) ? 1.0f : 0.72f - 0.08f * r;
                const float vel = juce::jlimit (0.02f, 1.0f, st.velocity * ls.gain * ratchetScale);
                const auto  v   = (juce::uint8) juce::jlimit (1, 127, (int) std::lround (vel * 127.0f));

                const double onTick = juce::jmax (0.0, tick);

                seq.addEvent (juce::MidiMessage::noteOn  (1, ls.midiNote, v), onTick);
                seq.addEvent (juce::MidiMessage::noteOff (1, ls.midiNote),    onTick + gate);
            }
        }
    }

    seq.updateMatchedPairs();
    seq.sort();

    return seq;
}

bool MidiExport::lanesShareNotes (const Kit& kit)
{
    for (int a = 0; a < kNumLanes; ++a)
    {
        if (! kit.lanes[(size_t) a].enabled)
            continue;

        for (int b = a + 1; b < kNumLanes; ++b)
            if (kit.lanes[(size_t) b].enabled
                && kit.lanes[(size_t) a].midiNote == kit.lanes[(size_t) b].midiNote)
                return true;
    }

    return false;
}

static juce::File writeMidiFile (const juce::MidiFile& file, const juce::String& baseName)
{
    auto out = juce::File::getSpecialLocation (juce::File::tempDirectory)
                   .getChildFile ("DRUMid")
                   .getChildFile (juce::File::createLegalFileName (baseName) + ".mid");

    out.getParentDirectory().createDirectory();
    out.deleteFile();

    if (auto stream = out.createOutputStream())
    {
        file.writeTo (*stream);
        stream->flush();
        return out;
    }

    return {};
}

juce::File MidiExport::writeMultiTrackTempFile (const Kit& kit, const juce::String& baseName)
{
    juce::MidiFile file;
    file.setTicksPerQuarterNote (kTicksPerQuarter);

    const double endTick = juce::jlimit (1, kMaxSteps, kit.numSteps) * kTicksPerStep;
    int tracksWritten = 0;

    for (int l = 0; l < kNumLanes; ++l)
    {
        if (! kit.lanes[(size_t) l].enabled)
            continue;

        auto seq = toSequence (kit, l);

        if (seq.getNumEvents() == 0)
            continue;

        // Naming the track is what makes Ableton label the created tracks
        // "Kick", "Clap" and so on instead of "1-MIDI", "2-MIDI".
        seq.addEvent (juce::MidiMessage::textMetaEvent (3, laneName ((LaneId) l)), 0.0);
        seq.addEvent (juce::MidiMessage::endOfTrack(), endTick);
        seq.sort();

        file.addTrack (seq);
        ++tracksWritten;
    }

    if (tracksWritten == 0)
        return {};

    return writeMidiFile (file, baseName);
}

juce::File MidiExport::writeKitTempFile (const Kit& kit, const juce::String& baseName)
{
    return lanesShareNotes (kit) ? writeMultiTrackTempFile (kit, baseName)
                                 : writeTempFile (kit, baseName, -1);
}

juce::File MidiExport::writeTempFile (const Kit& kit, const juce::String& baseName, int laneFilter)
{
    auto seq = toSequence (kit, laneFilter);

    if (seq.getNumEvents() == 0)
        return {};

    juce::MidiFile file;
    file.setTicksPerQuarterNote (kTicksPerQuarter);

    // Keep the clip exactly as long as the pattern so Live loops it correctly
    // instead of trimming to the last note.
    const double endTick = juce::jlimit (1, kMaxSteps, kit.numSteps) * kTicksPerStep;
    seq.addEvent (juce::MidiMessage::endOfTrack(), endTick);

    file.addTrack (seq);

    return writeMidiFile (file, baseName);
}

} // namespace drumid
