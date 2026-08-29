#include "Generator.h"
#include "PatternLibrary.h"

#include <algorithm>

namespace drummy
{

// ============================================================================
//  helpers
// ============================================================================

static inline float clamp (float v, float lo, float hi) noexcept
{
    return v < lo ? lo : (v > hi ? hi : v);
}

struct Rng
{
    explicit Rng (int seed) : gen ((std::mt19937::result_type) (seed * 2654435761u + 1u)) {}

    float uni()                 { return dist (gen); }                    // 0..1
    float bi()                  { return dist (gen) * 2.0f - 1.0f; }      // -1..1
    bool  chance (float p)      { return dist (gen) < p; }
    int   below (int n)         { return n <= 0 ? 0 : (int) (dist (gen) * (float) n) % n; }

    std::mt19937 gen;
    std::uniform_real_distribution<float> dist { 0.0f, 1.0f };
};

std::vector<int> Generator::euclid (int k, int n, int rotation)
{
    std::vector<int> out;

    if (n <= 0 || k <= 0)
        return out;

    k = std::min (k, n);

    for (int i = 0; i < n; ++i)
        if (((i * k) % n) < k)
            out.push_back ((i + rotation % n + n) % n);

    std::sort (out.begin(), out.end());
    return out;
}

// ============================================================================
//  Layer 1 - weighted seed pick
// ============================================================================

static LanePattern pickSeed (Genre genre, LaneId lane, float energy, int numSteps, Rng& rng)
{
    auto seeds = PatternLibrary::eligible (genre, lane, energy);

    if (seeds.empty())
    {
        LanePattern empty;
        empty.numSteps = numSteps;
        return empty;
    }

    float total = 0.0f;

    for (auto* s : seeds)
        total += s->weight;

    float pick = rng.uni() * total;

    for (auto* s : seeds)
    {
        pick -= s->weight;

        if (pick <= 0.0f)
            return PatternLibrary::fromGrid (s->grid, numSteps);
    }

    return PatternLibrary::fromGrid (seeds.back()->grid, numSteps);
}

// ============================================================================
//  Layer 2 - compatibility rules
//
//  This is the difference between "five lanes playing" and "a groove". Lanes
//  are generated top-down and each one reacts to what is already there.
// ============================================================================

static void applyCompatibility (Kit& kit, LaneId lane, const GenSettings& s, Rng& rng)
{
    const int idx    = (int) lane;
    const int n      = kit.numSteps;
    auto&     me     = kit.patterns[(size_t) idx];
    const auto& kick = kit.patterns[(size_t) LaneId::Kick];
    const auto& clap = kit.patterns[(size_t) LaneId::Clap];

    switch (lane)
    {
        case LaneId::OpenHat:
            // An open hat sitting on a kick is mud - the kick masks its whole
            // tail. Move it to the following 16th instead of dropping it.
            for (int i = 0; i < n; ++i)
            {
                if (me.steps[(size_t) i].on && kick.steps[(size_t) i].on)
                {
                    me.steps[(size_t) i].on = false;

                    if (i + 1 < n && ! me.steps[(size_t) (i + 1)].on)
                        me.steps[(size_t) (i + 1)] = Step { true, 0.85f, 0.0f, 1.0f, 1 };
                }
            }
            break;

        case LaneId::ClosedHat:
            // Duck the hat where the kick lands so the low end stays clean, and
            // duck it hard under the clap so the backbeat reads.
            for (int i = 0; i < n; ++i)
            {
                auto& st = me.steps[(size_t) i];

                if (! st.on)
                    continue;

                if (kick.steps[(size_t) i].on)
                    st.velocity *= 0.6f;

                if (clap.steps[(size_t) i].on && clap.steps[(size_t) i].velocity > 0.6f)
                    st.velocity *= 0.7f;
            }
            break;

        case LaneId::Shaker:
            // Percussion is the voice that should sit *between* the kicks. In
            // organic house that displacement is the whole point; in techno the
            // metal lives in the gaps. Melodic house is more forgiving.
            {
                const float avoid = (s.genre == Genre::OrganicHouse) ? 0.85f
                                  : (s.genre == Genre::Techno)       ? 0.7f
                                                                     : 0.35f;

                for (int i = 0; i < n; ++i)
                {
                    auto& st = me.steps[(size_t) i];

                    if (! st.on || ! kick.steps[(size_t) i].on)
                        continue;

                    if (rng.chance (avoid))
                    {
                        // Prefer to displace by a 16th; only ghost it if the
                        // neighbour is taken.
                        if (i + 1 < n && ! me.steps[(size_t) (i + 1)].on)
                        {
                            me.steps[(size_t) (i + 1)] = st;
                            st.on = false;
                        }
                        else
                        {
                            st.velocity *= 0.5f;
                        }
                    }
                }
            }
            break;

        default:
            break;
    }
}

// ============================================================================
//  Layer 3 - variation
// ============================================================================

static void applyDensity (LanePattern& p, LaneId lane, const GenSettings& s, Rng& rng)
{
    const int n = p.numSteps;

    // Thin out when Energy is low: ghosts go first, accents survive.
    if (s.energy < 0.4f)
    {
        const float drop = (0.4f - s.energy) * 1.6f;

        for (int i = 0; i < n; ++i)
        {
            auto& st = p.steps[(size_t) i];

            if (st.on && st.velocity < 0.6f && rng.chance (drop))
                st.on = false;
        }
    }

    // Fill in when Energy is high, but only on the lanes where extra hits are
    // musical - never on the kick or the clap.
    if (s.energy > 0.7f && (lane == LaneId::ClosedHat || lane == LaneId::Shaker))
    {
        const int extra = 1 + (int) ((s.energy - 0.7f) * 10.0f);
        const auto slots = Generator::euclid (extra * 2, 16, rng.below (4));

        for (int bar = 0; bar < n / kStepsPerBar; ++bar)
        {
            for (auto slot : slots)
            {
                const int i = bar * kStepsPerBar + slot;

                if (i < n && ! p.steps[(size_t) i].on && rng.chance (0.5f))
                    p.steps[(size_t) i] = Step { true, 0.35f + rng.uni() * 0.15f, 0.0f, 0.8f, 1 };
            }
        }
    }
}

static void applyGroove (LanePattern& p, LaneId lane, const GenSettings& s, Rng& rng)
{
    const int n = p.numSteps;

    // Swing: push the odd 16ths late. Stored as micro-timing on the step so it
    // is visible in the grid and survives the MIDI export.
    const float swingShift = (s.swing - 0.5f) * 2.0f * 0.5f;

    for (int i = 0; i < n; ++i)
    {
        auto& st = p.steps[(size_t) i];

        if (! st.on)
            continue;

        if (i % 2 == 1)
            st.micro += swingShift;

        // Humanize. The kick stays tight - dragging the kick just sounds broken.
        const float timingScale = (lane == LaneId::Kick) ? 0.25f : 1.0f;
        st.micro += rng.bi() * s.humanTiming * 0.12f * timingScale;
        st.micro  = clamp (st.micro, -0.45f, 0.45f);

        st.velocity *= 1.0f + rng.bi() * s.humanVel * 0.35f;

        // Downbeat reinforcement keeps the bar readable after humanizing.
        if (i % 4 == 0)
            st.velocity *= 1.08f;

        st.velocity = clamp (st.velocity, 0.05f, 1.0f);

        // Ghosts become probabilistic so repeats never sound identical.
        if (st.velocity < 0.5f)
            st.probability = clamp (1.0f - s.complexity * 0.55f, 0.35f, 1.0f);
    }

    // Ratchets: 32nd rolls, only on hats and percussion, weighted to the end of
    // the phrase where they read as momentum instead of noise.
    if (lane == LaneId::ClosedHat || lane == LaneId::Shaker)
    {
        for (int i = 0; i < n; ++i)
        {
            auto& st = p.steps[(size_t) i];

            if (! st.on || st.ratchet > 1)
                continue;

            const float positional = (float) (i % kStepsPerBar) / (float) kStepsPerBar;

            if (rng.chance (s.complexity * 0.10f * (0.3f + positional)))
                st.ratchet = rng.chance (0.75f) ? 2 : 3;
        }
    }
}

static void applyBarVariation (LanePattern& p, LaneId lane, const GenSettings& s, Rng& rng)
{
    const int bars = p.numSteps / kStepsPerBar;

    if (bars < 2)
        return;

    // Bar 1 is the statement; the bars after it answer. Only ghosts move -
    // accents and the kick's downbeats are the skeleton and stay put.
    const float amount = 0.18f + s.complexity * 0.45f;

    for (int bar = 1; bar < bars; ++bar)
    {
        const int off = bar * kStepsPerBar;

        for (int i = 0; i < kStepsPerBar; ++i)
        {
            auto& st = p.steps[(size_t) (off + i)];

            if (lane == LaneId::Kick && (i % 4) == 0)
                continue;

            if (st.on && st.velocity >= 0.7f)
                continue;

            if (! rng.chance (amount * 0.35f))
                continue;

            if (st.on)
            {
                // Displace the ghost by a 16th, or drop it entirely.
                const int to = i + (rng.chance (0.5f) ? 1 : -1);

                if (to >= 0 && to < kStepsPerBar && ! p.steps[(size_t) (off + to)].on)
                {
                    p.steps[(size_t) (off + to)] = st;
                    st.clear();
                }
                else
                {
                    st.clear();
                }
            }
            else if (lane != LaneId::Kick && lane != LaneId::Clap && (i % 4) != 0)
            {
                // Answer with a new ghost, off the beat only.
                st = Step { true, 0.32f + rng.uni() * 0.18f, 0.0f, 0.85f, 1 };
            }
        }
    }
}

static void applyFill (Kit& kit, LaneId lane, const GenSettings& s, Rng& rng)
{
    if (! s.fills || kit.bars() < 2)
        return;

    auto& p = kit.patterns[(size_t) lane];
    const int n = p.numSteps;
    const int lastBarStart = n - kStepsPerBar;

    switch (lane)
    {
        case LaneId::ClosedHat:
        case LaneId::Shaker:
            for (int i = lastBarStart + 12; i < n; ++i)
                if (rng.chance (0.45f + s.complexity * 0.3f))
                    p.steps[(size_t) i] = Step { true, 0.55f + rng.uni() * 0.35f, 0.0f, 1.0f,
                                                 rng.chance (0.3f) ? 2 : 1 };
            break;

        case LaneId::Clap:
            if (rng.chance (0.35f + s.complexity * 0.3f))
                p.steps[(size_t) (n - 2)] = Step { true, 0.7f, 0.0f, 1.0f, 1 };
            break;

        case LaneId::Kick:
            // Dropping the last kick is the oldest trick in the book and still
            // the most effective way to mark the end of a phrase.
            if (s.genre != Genre::Techno && rng.chance (0.2f))
                p.steps[(size_t) (lastBarStart + 12)].on = false;
            break;

        default:
            break;
    }
}

/** Non-negotiable genre rules, applied last so that fills and bar variation
    cannot quietly undo them. */
static void applyGenreGuards (LanePattern& p, LaneId lane, const GenSettings& s)
{
    if (s.genre != Genre::Techno || lane != LaneId::ClosedHat)
        return;

    // The offbeat-only hat is the sound of techno. A continuous 16th pattern is
    // the one legitimate exception - there the downbeat accent is the point - so
    // the rule only bites on the sparse patterns it was written for.
    const float density = (float) p.hitCount() / (float) (p.numSteps > 1 ? p.numSteps : 1);

    if (density >= 0.55f)
        return;

    for (int i = 0; i < p.numSteps; i += 4)
        p.steps[(size_t) i].clear();
}

// ============================================================================
//  entry points
// ============================================================================

void Generator::generateLane (Kit& kit, LaneId lane, const GenSettings& s, int laneSeed)
{
    const int idx = (int) lane;

    if (idx < 0 || idx >= kNumLanes)
        return;

    Rng rng (laneSeed);

    kit.setBars (s.bars);

    auto p = pickSeed (s.genre, lane, s.energy, kit.numSteps, rng);
    kit.patterns[(size_t) idx] = p;

    applyDensity (kit.patterns[(size_t) idx], lane, s, rng);
    applyCompatibility (kit, lane, s, rng);
    applyBarVariation (kit.patterns[(size_t) idx], lane, s, rng);
    applyFill (kit, lane, s, rng);
    applyGroove (kit.patterns[(size_t) idx], lane, s, rng);
    applyGenreGuards (kit.patterns[(size_t) idx], lane, s);
}

void Generator::generate (Kit& kit, const GenSettings& s)
{
    // Dependency order: everything downstream reads the kick, the hats read the
    // clap. Changing this order changes the groove, not just the code.
    static const LaneId order[] =
    {
        LaneId::Kick, LaneId::Clap, LaneId::OpenHat, LaneId::ClosedHat, LaneId::Shaker
    };

    kit.setBars (s.bars);

    for (auto lane : order)
    {
        const int idx = (int) lane;

        if (! kit.lanes[(size_t) idx].enabled || kit.lanes[(size_t) idx].locked)
            continue;

        generateLane (kit, lane, s, s.seed * 7919 + idx * 104729);
    }
}

} // namespace drummy
