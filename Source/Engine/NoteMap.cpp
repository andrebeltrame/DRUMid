#include "NoteMap.h"

namespace drummy
{

const char* NoteMap::presetName (NoteMapPreset p)
{
    switch (p)
    {
        case NoteMapPreset::GeneralMidi: return "GM / Drum Rack";
        case NoteMapPreset::DrumRack4x4: return "Drum Rack 4x4";
        case NoteMapPreset::Custom:      return "Custom";
        default:                         return "?";
    }
}

int NoteMap::noteFor (NoteMapPreset preset, LaneId lane)
{
    const int idx = (int) lane;

    if (preset == NoteMapPreset::DrumRack4x4)
        return 36 + idx;   // C1 upward, one pad per lane

    // General MIDI, which is also what Ableton's factory Drum Racks follow.
    switch (lane)
    {
        case LaneId::Kick:       return 36;  // C1  bass drum
        case LaneId::Clap:       return 39;  // D#1 hand clap
        case LaneId::Tom:        return 45;  // A1  low tom
        case LaneId::ClosedHat:  return 42;  // F#1 closed hi-hat
        case LaneId::OpenHat:    return 46;  // A#1 open hi-hat
        case LaneId::Shaker:     return 70;  // A#3 shaker
        case LaneId::Percussion: return 63;  // D#3 open high conga
        default:                 return 36 + idx;
    }
}

void NoteMap::apply (Kit& kit, NoteMapPreset preset)
{
    if (preset == NoteMapPreset::Custom)
        return;   // leave whatever the user dialled in

    for (int i = 0; i < kNumLanes; ++i)
        kit.lanes[(size_t) i].midiNote = noteFor (preset, (LaneId) i);
}

std::string NoteMap::noteName (int midiNote)
{
    static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

    if (midiNote < 0)
        midiNote = 0;

    if (midiNote > 127)
        midiNote = 127;

    // Ableton convention: MIDI 36 is C1, so octave = note/12 - 2.
    const int octave = midiNote / 12 - 2;

    return std::string (names[midiNote % 12]) + std::to_string (octave);
}

} // namespace drummy
