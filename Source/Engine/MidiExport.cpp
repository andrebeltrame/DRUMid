#include "MidiExport.h"

namespace drummy
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

        if (! ls.enabled || ls.muted)
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

    auto out = juce::File::getSpecialLocation (juce::File::tempDirectory)
                   .getChildFile ("Drummy")
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

} // namespace drummy
