#pragma once

#include <array>
#include <cmath>

namespace drummy
{

static constexpr int kStepsPerBar = 16;   // 16th-note grid
static constexpr int kMaxBars     = 4;
static constexpr int kMaxSteps    = kStepsPerBar * kMaxBars;

enum class Genre
{
    MelodicHouse = 0,
    OrganicHouse,
    Techno,
    NumGenres
};

/** Phase 1 lane set. The engine is written so adding lanes here plus seeds in
    PatternLibrary.cpp is all it takes to grow the kit. */
enum class LaneId
{
    Kick = 0,
    Clap,
    ClosedHat,
    OpenHat,
    Shaker,
    NumLanes
};

static constexpr int kNumLanes = static_cast<int> (LaneId::NumLanes);

inline const char* laneName (LaneId l) noexcept
{
    switch (l)
    {
        case LaneId::Kick:      return "Kick";
        case LaneId::Clap:      return "Clap";
        case LaneId::ClosedHat: return "Closed Hat";
        case LaneId::OpenHat:   return "Open Hat";
        case LaneId::Shaker:    return "Shaker";
        default:                return "?";
    }
}

inline const char* genreName (Genre g) noexcept
{
    switch (g)
    {
        case Genre::MelodicHouse: return "Melodic House";
        case Genre::OrganicHouse: return "Organic House";
        case Genre::Techno:       return "Techno";
        default:                  return "?";
    }
}

/** One 16th-note slot. */
struct Step
{
    bool  on          = false;
    float velocity    = 0.8f;   // 0..1
    float micro       = 0.0f;   // timing offset in fractions of a step, -0.5..0.5
    float probability = 1.0f;   // 0..1, rolled per playback cycle
    int   ratchet     = 1;      // 1 = single hit, 2..4 = roll inside the step

    void clear() noexcept { *this = Step {}; }
};

struct LanePattern
{
    std::array<Step, kMaxSteps> steps {};
    int numSteps = kStepsPerBar;

    void clear() noexcept
    {
        for (auto& s : steps)
            s.clear();
    }

    int hitCount() const noexcept
    {
        int n = 0;

        for (int i = 0; i < numSteps; ++i)
            if (steps[i].on)
                ++n;

        return n;
    }
};

/** Per-lane user settings that survive a re-generate. */
struct LaneSettings
{
    bool enabled  = true;
    bool locked   = false;   // locked lanes are never touched by Generate
    bool muted    = false;
    int  midiNote = 36;
    float gain    = 1.0f;    // velocity scaler, 0..1.5
};

struct Kit
{
    std::array<LanePattern,  kNumLanes> patterns {};
    std::array<LaneSettings, kNumLanes> lanes {};
    int numSteps = kStepsPerBar;

    void setBars (int bars) noexcept
    {
        bars     = bars < 1 ? 1 : (bars > kMaxBars ? kMaxBars : bars);
        numSteps = bars * kStepsPerBar;

        for (auto& p : patterns)
            p.numSteps = numSteps;
    }

    int bars() const noexcept { return numSteps / kStepsPerBar; }
};

/** The knobs the generator reads. */
struct GenSettings
{
    Genre genre       = Genre::MelodicHouse;
    float energy      = 0.5f;   // 0..1 - global density budget
    float complexity  = 0.4f;   // 0..1 - syncopation, ratchets, ghosts
    float swing       = 0.52f;  // 0.5 = straight, 0.667 = triplet
    float humanTiming = 0.15f;  // 0..1
    float humanVel    = 0.25f;  // 0..1
    int   bars        = 2;
    int   seed        = 1;
    bool  fills       = true;   // vary/fill on the last bar of the phrase
};

} // namespace drummy
