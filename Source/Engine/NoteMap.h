#pragma once

#include "Types.h"
#include <string>

namespace drummy
{

/** Where each lane lands on the keyboard.

    This is the make-or-break detail for Drum Racks: the pattern is only useful
    if it fires the right pad. Two factory maps cover almost every rack in the
    wild, and Custom lets you dial in a rack that was built by hand.
*/
enum class NoteMapPreset
{
    GeneralMidi = 0,   // 36 kick / 39 clap / 42 closed / 46 open / 70 shaker
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
