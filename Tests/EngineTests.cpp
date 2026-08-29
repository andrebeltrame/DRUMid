/*  Console checks for the parts that are hard to hear a bug in:
    the generator's output shape and the .mid file that the drag-out produces.

    cmake -B build -DDRUMMY_BUILD_TESTS=ON && cmake --build build --target DrummyTests
    ./build/DrummyTests
*/

#include "Engine/Generator.h"
#include "Engine/PatternLibrary.h"
#include "Engine/NoteMap.h"
#include "Engine/MidiExport.h"

#include <cstdio>

using namespace drummy;

static int failures = 0;

static void check (bool ok, const juce::String& what)
{
    std::printf ("  [%s] %s\n", ok ? "ok  " : "FAIL", what.toRawUTF8());

    if (! ok)
        ++failures;
}

static char glyph (const Step& s)
{
    if (! s.on)              return '.';
    if (s.ratchet > 1)       return s.ratchet == 2 ? ':' : '=';
    if (s.velocity >= 0.8f)  return 'X';
    if (s.velocity >= 0.55f) return 'x';
    return 'o';
}

static Kit makeKit (Genre g, float energy, float complexity, int seed, int bars)
{
    Kit kit;
    GenSettings s;
    s.genre = g; s.energy = energy; s.complexity = complexity;
    s.bars = bars; s.seed = seed;
    s.swing = (g == Genre::Techno) ? 0.50f : (g == Genre::OrganicHouse ? 0.56f : 0.53f);

    NoteMap::apply (kit, NoteMapPreset::GeneralMidi);
    Generator::generate (kit, s);
    return kit;
}

static void dump (const juce::String& title, const Kit& kit)
{
    std::printf ("\n--- %s\n", title.toRawUTF8());

    for (int l = 0; l < kNumLanes; ++l)
    {
        std::printf ("  %-11s %-4s |", laneName ((LaneId) l),
                     NoteMap::noteName (kit.lanes[(size_t) l].midiNote).c_str());

        for (int i = 0; i < kit.numSteps; ++i)
        {
            if (i > 0 && i % 4 == 0) std::printf ("|");
            std::printf ("%c", glyph (kit.patterns[(size_t) l].steps[(size_t) i]));
        }

        std::printf ("|\n");
    }
}

int main()
{
    std::printf ("Drummy engine checks - %d seeds in the bank\n", PatternLibrary::size());

    // ---- generator shape ---------------------------------------------------
    std::printf ("\ngenerator\n");
    {
        auto melodic = makeKit (Genre::MelodicHouse, 0.55f, 0.40f, 12345, 2);
        auto organic = makeKit (Genre::OrganicHouse, 0.55f, 0.55f, 777,   2);
        auto techno  = makeKit (Genre::Techno,       0.75f, 0.50f, 4242,  2);

        dump ("melodic house", melodic);
        dump ("organic house", organic);
        dump ("techno",        techno);
        std::printf ("\n");

        // Every genre is four-on-the-floor: a kick on each quarter, allowing for
        // the one the end-of-phrase fill is permitted to drop.
        for (auto* k : { &melodic, &organic, &techno })
        {
            int downbeats = 0;

            for (int i = 0; i < k->numSteps; i += 4)
                if (k->patterns[(size_t) LaneId::Kick].steps[(size_t) i].on)
                    ++downbeats;

            check (downbeats >= k->numSteps / 4 - 1, "kick lands on the quarters");
        }

        // Techno's identity: the closed hat stays off the downbeat.
        int technoDownbeatHats = 0;

        for (int i = 0; i < techno.numSteps; i += 4)
            if (techno.patterns[(size_t) LaneId::ClosedHat].steps[(size_t) i].on)
                ++technoDownbeatHats;

        const float hatDensity = (float) techno.patterns[(size_t) LaneId::ClosedHat].hitCount()
                                 / (float) techno.numSteps;

        // A continuous 16th hat is allowed its downbeat accent; every sparser
        // techno hat has to stay on the offbeat.
        check (hatDensity >= 0.55f || technoDownbeatHats == 0,
               "techno closed hat avoids the downbeat");

        // Bar 2 must not be a carbon copy of bar 1, or 2-bar patterns read as a
        // 1-bar loop.
        bool anyDifference = false;

        for (int l = 0; l < kNumLanes && ! anyDifference; ++l)
            for (int i = 0; i < kStepsPerBar; ++i)
                if (organic.patterns[(size_t) l].steps[(size_t) i].on
                    != organic.patterns[(size_t) l].steps[(size_t) (i + kStepsPerBar)].on)
                { anyDifference = true; break; }

        check (anyDifference, "bar 2 differs from bar 1");

        // Swing has to be real micro-timing, not a quantised lie.
        float maxMicro = 0.0f;

        for (int l = 0; l < kNumLanes; ++l)
            for (int i = 0; i < organic.numSteps; ++i)
                if (organic.patterns[(size_t) l].steps[(size_t) i].on)
                    maxMicro = juce::jmax (maxMicro, std::abs (organic.patterns[(size_t) l].steps[(size_t) i].micro));

        check (maxMicro > 0.02f, "swing and humanize produce real micro-timing");

        // Lock must survive a regenerate.
        Kit locked = melodic;
        GenSettings s; s.genre = Genre::MelodicHouse; s.bars = 2; s.seed = 999;
        locked.lanes[(size_t) LaneId::Kick].locked = true;
        auto before = locked.patterns[(size_t) LaneId::Kick];
        Generator::generate (locked, s);

        bool kickUntouched = true;

        for (int i = 0; i < locked.numSteps; ++i)
            if (before.steps[(size_t) i].on != locked.patterns[(size_t) LaneId::Kick].steps[(size_t) i].on)
            { kickUntouched = false; break; }

        check (kickUntouched, "a locked lane survives GENERATE");

        // Same seed, same kit.
        Kit a, b;
        GenSettings d; d.seed = 555; d.genre = Genre::OrganicHouse;
        Generator::generate (a, d);
        Generator::generate (b, d);

        bool identical = true;

        for (int l = 0; l < kNumLanes && identical; ++l)
            for (int i = 0; i < a.numSteps; ++i)
                if (a.patterns[(size_t) l].steps[(size_t) i].on != b.patterns[(size_t) l].steps[(size_t) i].on
                    || std::abs (a.patterns[(size_t) l].steps[(size_t) i].velocity
                                 - b.patterns[(size_t) l].steps[(size_t) i].velocity) > 1.0e-6f)
                { identical = false; break; }

        check (identical, "the same seed rebuilds the same kit");
    }

    // ---- the MIDI file the drag-out hands to Ableton ------------------------
    std::printf ("\nmidi export\n");
    {
        auto kit = makeKit (Genre::MelodicHouse, 0.6f, 0.4f, 2024, 2);

        auto file = MidiExport::writeTempFile (kit, "DrummyTest_kit", -1);
        check (file.existsAsFile() && file.getSize() > 0, "kit .mid is written");

        juce::FileInputStream in (file);
        juce::MidiFile read;
        check (read.readFrom (in), "the .mid reads back as a valid MIDI file");
        check (read.getNumTracks() == 1, "one track");
        check (read.getTimeFormat() == 960, "960 ticks per quarter");

        if (read.getNumTracks() > 0)
        {
            const auto* track = read.getTrack (0);

            int noteOns = 0;
            int expected = 0;

            for (int i = 0; i < track->getNumEvents(); ++i)
                if (track->getEventPointer (i)->message.isNoteOn())
                    ++noteOns;

            for (int l = 0; l < kNumLanes; ++l)
            {
                if (! kit.lanes[(size_t) l].enabled || kit.lanes[(size_t) l].muted)
                    continue;

                for (int i = 0; i < kit.numSteps; ++i)
                    if (kit.patterns[(size_t) l].steps[(size_t) i].on)
                        expected += juce::jlimit (1, 4, kit.patterns[(size_t) l].steps[(size_t) i].ratchet);
            }

            std::printf ("       %d note-ons written, %d expected\n", noteOns, expected);
            check (noteOns == expected, "every hit and ratchet made it into the file");

            // The clip has to be exactly as long as the pattern or Live trims it
            // to the last note and the loop stops lining up.
            const double patternTicks = kit.numSteps * 960 * 0.25;
            check (std::abs (track->getEndTime() - patternTicks) < 1.0,
                   "the clip is exactly " + juce::String (kit.bars()) + " bars long");

            // Notes must land on the lanes' mapped pads, not somewhere random.
            bool notesInMap = true;

            for (int i = 0; i < track->getNumEvents(); ++i)
            {
                const auto& m = track->getEventPointer (i)->message;

                if (! m.isNoteOnOrOff())
                    continue;

                bool found = false;

                for (int l = 0; l < kNumLanes; ++l)
                    if (m.getNoteNumber() == kit.lanes[(size_t) l].midiNote)
                        found = true;

                if (! found)
                    notesInMap = false;
            }

            check (notesInMap, "every note lands on a mapped pad");
        }

        // Single-lane drag.
        auto kickOnly = MidiExport::writeTempFile (kit, "DrummyTest_kick", (int) LaneId::Kick);
        juce::FileInputStream in2 (kickOnly);
        juce::MidiFile readKick;
        readKick.readFrom (in2);

        bool onlyKickNotes = readKick.getNumTracks() > 0;

        if (onlyKickNotes)
            for (int i = 0; i < readKick.getTrack (0)->getNumEvents(); ++i)
            {
                const auto& m = readKick.getTrack (0)->getEventPointer (i)->message;

                if (m.isNoteOnOrOff() && m.getNoteNumber() != kit.lanes[(size_t) LaneId::Kick].midiNote)
                    onlyKickNotes = false;
            }

        check (onlyKickNotes, "a single-lane drag exports that lane only");

        file.deleteFile();
        kickOnly.deleteFile();
    }

    std::printf ("\n%s (%d failure%s)\n", failures == 0 ? "PASS" : "FAIL",
                 failures, failures == 1 ? "" : "s");

    return failures == 0 ? 0 : 1;
}
