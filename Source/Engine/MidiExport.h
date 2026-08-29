#pragma once

#include "Types.h"

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_core/juce_core.h>

namespace drummy
{

/** Turns a kit into a .mid file you can drag straight into an Ableton clip slot.

    Swing and humanize are baked into the exported timing, so what you drag out
    is exactly what you were hearing - not a quantised approximation of it.
*/
namespace MidiExport
{
    /** @param laneFilter  -1 for the whole kit, or a LaneId index for one lane. */
    juce::MidiMessageSequence toSequence (const Kit& kit, int laneFilter = -1);

    /** Writes to the temp folder and returns the file. Empty file on failure. */
    juce::File writeTempFile (const Kit& kit, const juce::String& baseName, int laneFilter = -1);
}

} // namespace drummy
