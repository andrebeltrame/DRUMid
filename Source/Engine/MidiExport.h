#pragma once

#include "Types.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

namespace drumid
{

/** Turns a kit into a .mid file you can drag straight into an Ableton clip slot.

    Swing and humanize are baked into the exported timing, so what you drag out
    is exactly what you were hearing - not a quantised approximation of it.
*/
namespace MidiExport
{
    /** @param laneFilter  -1 for the whole kit, or a LaneId index for one lane. */
    juce::MidiMessageSequence toSequence (const Kit& kit, int laneFilter = -1);

    /** True when two enabled lanes sit on the same note - which is the normal
        state under the single-note map, and means a flat one-track export would
        collapse the whole kit onto one voice. */
    bool lanesShareNotes (const Kit& kit);

    /** Writes to the temp folder and returns the file. Empty file on failure. */
    juce::File writeTempFile (const Kit& kit, const juce::String& baseName, int laneFilter = -1);

    /** The whole kit, as one track per lane.

        Dropped into Ableton's Session View this creates one track per
        instrument, which is what you want when every lane is going to its own
        Simpler rather than to a shared Drum Rack. */
    juce::File writeMultiTrackTempFile (const Kit& kit, const juce::String& baseName);

    /** Picks the right one: multi-track when a flat export would collapse lanes
        onto the same note, single track otherwise. */
    juce::File writeKitTempFile (const Kit& kit, const juce::String& baseName);
}

} // namespace drumid
