#pragma once

#include "Types.h"
#include <random>
#include <vector>

namespace drumid
{

/** The three-layer pattern brain.

    Layer 1  curated seed pick   (PatternLibrary, weighted by Energy)
    Layer 2  compatibility rules (lanes are generated in dependency order and
                                  react to what the lanes above them played)
    Layer 3  variation           (swing, humanize, probability, ratchets, fills)

    Everything is driven by GenSettings::seed, so the same seed number always
    rebuilds the same kit. That is what makes lock + reroll usable: you keep the
    kick you liked, bump the seed, and only the unlocked lanes change.
*/
namespace Generator
{
    /** Regenerate every enabled, unlocked lane. */
    void generate (Kit& kit, const GenSettings& settings);

    /** Regenerate a single lane (used by the per-lane reroll button). */
    void generateLane (Kit& kit, LaneId lane, const GenSettings& settings, int laneSeed);

    /** Bjorklund / Euclidean rhythm: k onsets spread as evenly as possible over
        n steps, optionally rotated. Returns step indices. */
    std::vector<int> euclid (int k, int n, int rotation = 0);
}

} // namespace drumid
