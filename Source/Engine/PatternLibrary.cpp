#include "PatternLibrary.h"

#include <cstring>

namespace drummy
{

// ============================================================================
//  The curated bank.
//
//  Everything here is genre-specific on purpose. The signature of each style
//  lives in exactly these placements:
//    - melodic house : open hat on the 8th-note offbeats, 16th hats with an
//                      offbeat accent, light swing
//    - organic house : hand percussion carries the groove - tresillo, son and
//                      rumba clave, E(5,16)/E(7,16) - clap often only on 4
//    - techno        : closed hat on the offbeat only ("tsk"), zero swing,
//                      sparse syncopated metal, 16th ratchet rolls
// ============================================================================

static const Seed kSeeds[] =
{
    // ---------------------------------------------------------------- MELODIC HOUSE
    { Genre::MelodicHouse, LaneId::Kick,      "four on the floor", "X...X...X...X...",                 0.0f, 1.0f, 3.0f },
    { Genre::MelodicHouse, LaneId::Kick,      "ghost pickup",      "X...X...X...X..o",                 0.3f, 1.0f, 1.5f },
    { Genre::MelodicHouse, LaneId::Kick,      "bar-end lift",      "X...X...X...X...X...X...X...X..x", 0.4f, 1.0f, 1.0f },

    { Genre::MelodicHouse, LaneId::Clap,      "backbeat",          "....X.......X...",                 0.0f, 1.0f, 3.0f },
    { Genre::MelodicHouse, LaneId::Clap,      "backbeat + ghost",  "....X.......X..o",                 0.4f, 1.0f, 1.2f },
    { Genre::MelodicHouse, LaneId::Clap,      "push on 4",         "....X.......X.x.",                 0.5f, 1.0f, 0.8f },

    { Genre::MelodicHouse, LaneId::ClosedHat, "offbeat 8ths",      "..x...x...x...x.",                 0.0f, 0.6f, 2.0f },
    { Genre::MelodicHouse, LaneId::ClosedHat, "16ths offbeat acc", "xoXoxoXoxoXoxoXo",                 0.3f, 1.0f, 2.5f },
    { Genre::MelodicHouse, LaneId::ClosedHat, "16ths soft",        "oxoxoxoxoxoxoxox",                 0.2f, 0.8f, 1.5f },
    { Genre::MelodicHouse, LaneId::ClosedHat, "rolling w/ ratchet","xoXoxoXoxoXoxo:o",                 0.6f, 1.0f, 1.0f },

    { Genre::MelodicHouse, LaneId::OpenHat,   "offbeat 8ths",      "..X...X...X...X.",                 0.0f, 1.0f, 3.0f },
    { Genre::MelodicHouse, LaneId::OpenHat,   "and of 2 and 4",    "......X.......X.",                 0.0f, 0.6f, 1.5f },
    { Genre::MelodicHouse, LaneId::OpenHat,   "bar lift only",     "..............X.",                 0.0f, 0.4f, 1.0f },

    { Genre::MelodicHouse, LaneId::Shaker,    "16ths",             "oxoxoxoxoxoxoxox",                 0.3f, 1.0f, 2.0f },
    { Genre::MelodicHouse, LaneId::Shaker,    "offbeat 16ths",     ".x.x.x.x.x.x.x.x",                 0.0f, 0.8f, 1.5f },
    { Genre::MelodicHouse, LaneId::Shaker,    "tresillo",          "X..x..x.X..x..x.",                 0.4f, 1.0f, 1.2f },

    // ---------------------------------------------------------------- ORGANIC HOUSE
    { Genre::OrganicHouse, LaneId::Kick,      "soft four",         "X...X...X...X...",                 0.0f, 1.0f, 3.0f },
    { Genre::OrganicHouse, LaneId::Kick,      "with ghost tail",   "X...X...X...X.o.",                 0.3f, 1.0f, 1.5f },
    { Genre::OrganicHouse, LaneId::Kick,      "breathing",         "X..oX...X...X..o",                 0.5f, 1.0f, 1.0f },

    { Genre::OrganicHouse, LaneId::Clap,      "only on 4",         "............X...",                 0.0f, 0.7f, 2.5f },
    { Genre::OrganicHouse, LaneId::Clap,      "ghost 2, hit 4",    "....o.......X...",                 0.2f, 1.0f, 2.0f },
    { Genre::OrganicHouse, LaneId::Clap,      "backbeat",          "....X.......X...",                 0.4f, 1.0f, 1.5f },

    { Genre::OrganicHouse, LaneId::ClosedHat, "E(5,16)",           "x..x..x...x..x..",                 0.0f, 1.0f, 2.0f },
    { Genre::OrganicHouse, LaneId::ClosedHat, "loose 16ths",       "oxo.oxo.oxo.oxo.",                 0.3f, 1.0f, 2.0f },
    { Genre::OrganicHouse, LaneId::ClosedHat, "offbeat sparse",    "..x.....x.....x.",                 0.0f, 0.5f, 1.5f },
    { Genre::OrganicHouse, LaneId::ClosedHat, "hand 16ths",        "oXooxXoooXooxXoo",                 0.5f, 1.0f, 1.2f },

    { Genre::OrganicHouse, LaneId::OpenHat,   "one per bar",       "......X.........",                 0.0f, 0.6f, 2.0f },
    { Genre::OrganicHouse, LaneId::OpenHat,   "two per bar",       "..X.......X.....",                 0.3f, 1.0f, 1.5f },
    { Genre::OrganicHouse, LaneId::OpenHat,   "offbeat",           "..X...X...X...X.",                 0.6f, 1.0f, 0.8f },

    { Genre::OrganicHouse, LaneId::Shaker,    "tresillo",          "X..x..X.x..X..x.",                 0.0f, 1.0f, 2.5f },
    { Genre::OrganicHouse, LaneId::Shaker,    "son clave 3-2",     "X..X..X...X.X...",                 0.0f, 1.0f, 2.5f },
    { Genre::OrganicHouse, LaneId::Shaker,    "rumba clave 3-2",   "X..X...X..X.X...",                 0.2f, 1.0f, 2.0f },
    { Genre::OrganicHouse, LaneId::Shaker,    "E(7,16)",           "x..x.x.x..x.x.x.",                 0.3f, 1.0f, 2.0f },
    { Genre::OrganicHouse, LaneId::Shaker,    "hand 16ths",        "oXooxXoooXooxXoo",                 0.6f, 1.0f, 1.5f },

    // ---------------------------------------------------------------- TECHNO
    { Genre::Techno,       LaneId::Kick,      "straight four",     "X...X...X...X...",                 0.0f, 1.0f, 4.0f },
    { Genre::Techno,       LaneId::Kick,      "peak-time push",    "X...X...X...X.x.",                 0.6f, 1.0f, 1.0f },

    { Genre::Techno,       LaneId::Clap,      "backbeat",          "....X.......X...",                 0.0f, 1.0f, 3.0f },
    { Genre::Techno,       LaneId::Clap,      "backbeat + roll",   "....X.......X.:.",                 0.6f, 1.0f, 1.0f },
    { Genre::Techno,       LaneId::Clap,      "on 3 only",         "........X.......",                 0.0f, 0.4f, 1.0f },

    { Genre::Techno,       LaneId::ClosedHat, "offbeat tsk",       "..X...X...X...X.",                 0.0f, 1.0f, 3.0f },
    { Genre::Techno,       LaneId::ClosedHat, "16ths accent 4",    "XoooxoooXoooxooo",                 0.4f, 1.0f, 2.0f },
    { Genre::Techno,       LaneId::ClosedHat, "hypnotic 16ths",    "xoxoxoxoxoxoxoxo",                 0.5f, 1.0f, 1.8f },
    { Genre::Techno,       LaneId::ClosedHat, "offbeat + ratchet", "..X...X...X...:.",                 0.6f, 1.0f, 1.2f },

    { Genre::Techno,       LaneId::OpenHat,   "offbeat",           "..X...X...X...X.",                 0.3f, 1.0f, 2.5f },
    { Genre::Techno,       LaneId::OpenHat,   "bar lift",          "..............X.",                 0.0f, 0.7f, 1.5f },
    { Genre::Techno,       LaneId::OpenHat,   "and of 4 only",     "..............X.",                 0.0f, 0.5f, 1.0f },

    { Genre::Techno,       LaneId::Shaker,    "sparse metal",      "...x......x.....",                 0.0f, 0.8f, 2.0f },
    { Genre::Techno,       LaneId::Shaker,    "syncopated pair",   "......x.......x.",                 0.0f, 0.8f, 2.0f },
    { Genre::Techno,       LaneId::Shaker,    "16th ride",         "xxxxxxxxxxxxxxxx",                 0.6f, 1.0f, 1.5f },
    { Genre::Techno,       LaneId::Shaker,    "3-against-4",       "x..x..x..x..x...",                 0.4f, 1.0f, 1.5f },
};

static constexpr int kNumSeeds = (int) (sizeof (kSeeds) / sizeof (kSeeds[0]));

int PatternLibrary::size() { return kNumSeeds; }

std::vector<const Seed*> PatternLibrary::eligible (Genre genre, LaneId lane, float energy)
{
    std::vector<const Seed*> out;

    for (auto& s : kSeeds)
        if (s.genre == genre && s.lane == lane && energy >= s.energyMin && energy <= s.energyMax)
            out.push_back (&s);

    if (out.empty())
        for (auto& s : kSeeds)
            if (s.genre == genre && s.lane == lane)
                out.push_back (&s);

    return out;
}

LanePattern PatternLibrary::fromGrid (const char* grid, int numSteps)
{
    LanePattern p;
    p.numSteps = numSteps;

    const int len = grid != nullptr ? (int) std::strlen (grid) : 0;

    if (len == 0)
        return p;

    for (int i = 0; i < numSteps; ++i)
    {
        const char c = grid[i % len];
        auto& step = p.steps[(size_t) i];

        switch (c)
        {
            case 'X': step.on = true; step.velocity = 1.00f; break;
            case 'x': step.on = true; step.velocity = 0.75f; break;
            case 'o': step.on = true; step.velocity = 0.45f; break;
            case ':': step.on = true; step.velocity = 0.70f; step.ratchet = 2; break;
            case '=': step.on = true; step.velocity = 0.70f; step.ratchet = 3; break;
            default:  step.on = false; break;
        }
    }

    return p;
}

} // namespace drummy
