#pragma once

#include "Types.h"
#include <vector>

namespace drummy
{

/** A curated pattern seed.

    The grid is a step string, 16 or 32 chars long, tiled to fill the kit length:
      '.'  off
      'o'  ghost  (soft)
      'x'  normal
      'X'  accent
      ':'  normal hit with a 2-hit ratchet (32nd roll inside the step)
      '='  normal hit with a 3-hit ratchet

    energyMin/energyMax gate which seeds are eligible at the current Energy
    setting, so turning Energy up genuinely reaches for busier material instead
    of just adding random hits on top of a sparse pattern.
*/
struct Seed
{
    Genre       genre;
    LaneId      lane;
    const char* name;
    const char* grid;
    float       energyMin;
    float       energyMax;
    float       weight;
};

namespace PatternLibrary
{
    /** All seeds for a lane in a genre that are eligible at this energy.
        Never returns empty: falls back to the full lane set, then to a default. */
    std::vector<const Seed*> eligible (Genre genre, LaneId lane, float energy);

    /** Parse a step string into a pattern, tiling it out to numSteps. */
    LanePattern fromGrid (const char* grid, int numSteps);

    /** Total seed count, for diagnostics. */
    int size();
}

} // namespace drummy
