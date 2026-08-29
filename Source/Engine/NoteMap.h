#pragma once

#include "Types.h"
#include <string>

namespace drummy
{

/** Where each lane lands on the keyboard.

    Two workflows, two answers.

    Driving a Drum Rack, the note *is* what separates the instruments, so every
    lane needs its own note and the pattern is only useful if it fires the right
    pad. That is what the GM and 4x4 maps are for.

    Driving one Simpler per track, the routing separates the instruments and the
    note carries no information at all - so every lane sits on C3, the root note
    Simpler and Sampler default to. That is SingleNoteC3, and it is the default
    because it is the workflow this plugin was built around: generate, then drag
    each lane to its own track.
*/
enum class NoteMapPreset
{
    SingleNoteC3 = 0,  // every lane on C3 - one instrument per track
    GeneralMidi,       // 36 kick / 39 clap / 45 tom / 42 closed / 46 open / ...
    DrumRack4x4,       // C1 upward, chromatic - matches a rack laid out by pad
    Custom,
    NumPresets
};

namespace NoteMap
{
    const char* presetName (NoteMapPreset p);

    /** Note number this preset assigns to a lane. Custom returns the GM value
        as a starting point; the kit's own per-lane notes take over from there. */
    int noteFor (NoteMapPreset preset, LaneId lane);

    /** Stamp a preset onto the kit's lane settings. */
    void apply (Kit& kit, NoteMapPreset preset);

    /** "C1", "D#1", ... using Ableton's convention where note 36 is C1. */
    std::string noteName (int midiNote);
}

} // namespace drummy
