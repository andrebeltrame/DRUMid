#include "Generator.h"
#include "PatternLibrary.h"

#include <algorithm>

namespace drumid
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

        case LaneId::Tom:
        {
            // A tom is allowed to hit with the kick. Drummers do it constantly,
            // and three things here depend on it: the tribal tom reinforcing the
            // downbeat, a fill running straight through the beat, and techno's
            // rolling 8th tom - half of whose hits land on a kick by definition.
            //
            // The one case that genuinely does not work is a long low tom on
            // every kick in a track where the kick carries the sub. So the rule
            // is narrow rather than blanket: rolling patterns are left alone,
            // the bar downbeat is always reinforcement, and only the sub-heavy
            // genres thin what is left.
            const int bars = (n / kStepsPerBar) > 0 ? (n / kStepsPerBar) : 1;
            const float perBar = (float) me.hitCount() / (float) bars;

            const bool rolling = perBar >= 6.0f;

            const bool subHeavyKick = s.genre == Genre::MelodicHouse
                                   || s.genre == Genre::MelodicTechno
                                   || s.genre == Genre::Techno;

            auto displaceOrDrop = [&] (int i)
            {
                auto& st = me.steps[(size_t) i];

                if (i + 2 < n && ! me.steps[(size_t) (i + 2)].on)
                {
                    me.steps[(size_t) (i + 2)] = st;
                    st.clear();
                }
                else
                {
                    st.clear();
                }
            };

            for (int i = 0; i < n; ++i)
            {
                auto& st = me.steps[(size_t) i];

                if (! st.on)
                    continue;

                // Tom and clap on the same 16th is the one collision that reads
                // as a mistake rather than as a layer.
                if (clap.steps[(size_t) i].on && rng.chance (0.75f))
                {
                    displaceOrDrop (i);
                    continue;
                }

                if (! kick.steps[(size_t) i].on)
                    continue;

                // Reinforcement, not collision - duck it slightly where the kick
                // owns the low end, and otherwise let it hit.
                if (rolling || (i % kStepsPerBar) == 0)
                {
                    if (subHeavyKick)
                        st.velocity *= 0.88f;

                    continue;
                }

                if (! subHeavyKick)
                {
                    st.velocity *= 0.85f;
                    continue;
                }

                if (rng.chance (0.7f))
                    displaceOrDrop (i);
            }

            break;
        }

        case LaneId::Percussion:
        case LaneId::Percussion2:
            // Two hand voices playing the same rhythm is one voice's worth of
            // groove for two voices' worth of mud. The family generates shaker,
            // then percussion, then perc 2, so each one moves out of the way of
            // whatever is already there.
            {
                static const LaneId family[] =
                {
                    LaneId::Shaker, LaneId::Percussion, LaneId::Percussion2
                };

                for (auto other : family)
                {
                    if (other == lane)
                        break;   // only dodge the voices generated before this one

                    if (! kit.lanes[(size_t) other].enabled)
                        continue;

                    const auto& earlier = kit.patterns[(size_t) other];

                    for (int i = 0; i < n; ++i)
                    {
                        auto& st = me.steps[(size_t) i];

                        if (! st.on || ! earlier.steps[(size_t) i].on)
                            continue;

                        if (! rng.chance (0.65f))
                            continue;

                        if (i + 1 < n && ! me.steps[(size_t) (i + 1)].on
                            && ! earlier.steps[(size_t) (i + 1)].on)
                        {
                            me.steps[(size_t) (i + 1)] = st;
                            st.clear();
                        }
                        else
                        {
                            st.clear();
                        }
                    }
                }
            }
            [[fallthrough]];

        case LaneId::Shaker:
            // Percussion is the voice that should sit *between* the kicks. In
            // organic house that displacement is the whole point; in techno the
            // metal lives in the gaps. Melodic house is more forgiving.
            {
                float avoid = 0.35f;

                switch (s.genre)
                {
                    case Genre::OrganicHouse:  avoid = 0.85f; break;
                    case Genre::AfroHouse:     avoid = 0.85f; break;   // the perc IS the track
                    case Genre::MelodicTechno: avoid = 0.75f; break;
                    case Genre::Techno:        avoid = 0.70f; break;
                    case Genre::IndieDance:    avoid = 0.45f; break;   // played, not placed
                    case Genre::Cinematic:     avoid = 0.55f; break;
                    case Genre::ProgressiveHouse: avoid = 0.40f; break;
                    case Genre::BigRoomEDM:    avoid = 0.35f; break;
                    default:                   avoid = 0.35f; break;
                }

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
    if (s.energy > 0.7f && (lane == LaneId::ClosedHat || lane == LaneId::Shaker
                            || lane == LaneId::Percussion))
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

static void applyGroove (LanePattern& p, LaneId lane, const GenSettings& s, Rng& rng,
                         float laneDynamics)
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

        // The per-lane amount is what stops every element breathing by exactly
        // the same amount, which is the tell of a programmed kit.
        st.velocity *= 1.0f + rng.bi() * s.humanVel * laneDynamics * 0.35f;

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
    if (lane == LaneId::ClosedHat || lane == LaneId::Shaker || lane == LaneId::Percussion)
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

static void applyBarVariation (LanePattern& p, LaneId lane, const GenSettings& s, Rng& rng,
                               bool mayAddGhosts)
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
            else if (mayAddGhosts && lane != LaneId::Kick && lane != LaneId::Clap && (i % 4) != 0)
            {
                // Answer with a new ghost, off the beat only - but never on a
                // lane this genre wants quiet. Filling the gaps in a cinematic
                // hi-hat removes the exact thing the style is made of.
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
        case LaneId::Tom:
            // The tom fill is the classic end-of-phrase move: a run into the
            // downbeat rather than scattered hits.
            if (rng.chance (0.3f + s.complexity * 0.4f))
                for (int i = n - 4; i < n; ++i)
                    p.steps[(size_t) i] = Step { true,
                                                 0.5f + 0.15f * (float) (i - (n - 4)),
                                                 0.0f, 1.0f, 1 };
            break;

        case LaneId::ClosedHat:
        case LaneId::Shaker:
        case LaneId::Percussion:
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
            if (s.genre != Genre::Techno && s.genre != Genre::MelodicTechno && rng.chance (0.2f))
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
    const bool technoFamily = s.genre == Genre::Techno || s.genre == Genre::MelodicTechno;

    if (! technoFamily || lane != LaneId::ClosedHat)
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

/** How much a lane matters to a genre, used to decide who gives up hits when
    the kit is over budget.

    Without this the budget just thins whoever is busiest, which means the hi-hat
    - naturally the densest lane - starves the congas in afro house. That is
    exactly backwards: there the percussion is the lead voice and the hat is the
    one that should step back.
*/
static float laneImportance (Genre genre, LaneId lane)
{
    switch (genre)
    {
        case Genre::OrganicHouse:
        case Genre::AfroHouse:
            switch (lane)
            {
                case LaneId::Percussion:  return 2.0f;   // the lead voice
                case LaneId::Percussion2: return 1.7f;
                case LaneId::Shaker:      return 1.6f;
                case LaneId::Tom:        return 1.1f;
                case LaneId::OpenHat:    return 0.9f;
                case LaneId::ClosedHat:  return 0.7f;   // steps back first
                default:                 return 1.0f;
            }

        case Genre::IndieDance:
            switch (lane)
            {
                case LaneId::ClosedHat:   return 1.4f;
                case LaneId::OpenHat:     return 1.1f;
                case LaneId::Percussion:  return 1.1f;
                case LaneId::Percussion2: return 1.0f;
                default:                  return 1.0f;
            }

        case Genre::Cinematic:
            // The toms carry this one the way the hi-hat carries the others, so
            // the hats are the first thing to step back when the kit is full.
            switch (lane)
            {
                case LaneId::Tom:         return 2.2f;
                case LaneId::Percussion:  return 1.4f;
                case LaneId::Percussion2: return 1.1f;
                case LaneId::Shaker:      return 0.7f;
                case LaneId::ClosedHat:   return 0.5f;
                case LaneId::OpenHat:     return 0.5f;
                default:                  return 1.0f;
            }

        case Genre::ProgressiveHouse:
            switch (lane)
            {
                case LaneId::OpenHat:     return 1.4f;   // the offbeat hat is the engine
                case LaneId::ClosedHat:   return 1.4f;
                case LaneId::Shaker:      return 1.1f;
                case LaneId::Percussion:  return 1.0f;
                case LaneId::Tom:         return 0.8f;
                default:                  return 1.0f;
            }

        case Genre::BigRoomEDM:
            switch (lane)
            {
                case LaneId::ClosedHat:   return 1.5f;
                case LaneId::OpenHat:     return 1.3f;
                case LaneId::Tom:         return 0.9f;
                default:                  return 0.8f;   // nothing crowds the drop
            }

        case Genre::MelodicHouse:
            switch (lane)
            {
                case LaneId::ClosedHat:  return 1.4f;
                case LaneId::OpenHat:    return 1.3f;
                case LaneId::Shaker:     return 1.1f;
                case LaneId::Percussion: return 0.9f;
                case LaneId::Tom:        return 0.8f;
                default:                 return 1.0f;
            }

        case Genre::MelodicTechno:
        case Genre::Techno:
            switch (lane)
            {
                case LaneId::ClosedHat:  return 1.5f;   // the offbeat hat is the genre
                case LaneId::OpenHat:    return 1.1f;
                default:                 return 0.9f;
            }

        default:
            return 1.0f;
    }
}

/** Energy has to *distribute* hits, not stack them.

    Without this, every colour lane independently answers "how busy should I be?"
    and at seven lanes the honest answer from each one adds up to mud. The kick
    and the clap are the skeleton and are never touched; everything else competes
    for one shared budget, and the busiest lane gives up its quietest hit first.
*/
static void applyEnergyBudget (Kit& kit, const GenSettings& s, int onlyLane = -1)
{
    static const LaneId colour[] =
    {
        LaneId::Tom, LaneId::ClosedHat, LaneId::OpenHat,
        LaneId::Shaker, LaneId::Percussion, LaneId::Percussion2
    };

    const int bars = kit.numSteps / kStepsPerBar;

    // Cinematic is not a busier or quieter version of the others - it is built
    // out of space, so the same Energy has to buy far fewer hits there.
    const float genreScale = (s.genre == Genre::Cinematic) ? 0.55f : 1.0f;
    const int budget = (int) ((6.0f + s.energy * 32.0f) * (float) bars * genreScale);

    auto totalHits = [&]
    {
        int t = 0;

        for (auto l : colour)
            if (kit.lanes[(size_t) l].enabled)
                t += kit.patterns[(size_t) l].hitCount();

        return t;
    };

    // A lane is never thinned below this, so a lead voice cannot be silenced
    // into a fill-only lane by a busier neighbour.
    const int floorHits = bars * 2;

    for (int guard = 0; totalHits() > budget && guard < 512; ++guard)
    {
        int worst = -1;
        float worstScore = 0.0f;

        for (auto l : colour)
        {
            const int idx = (int) l;

            if (onlyLane >= 0 && idx != onlyLane)
                continue;

            if (! kit.lanes[(size_t) idx].enabled || kit.lanes[(size_t) idx].locked)
                continue;

            const int hits = kit.patterns[(size_t) idx].hitCount();

            if (hits <= floorHits)
                continue;

            // Over-represented relative to how much this lane matters here.
            const float score = (float) hits / laneImportance (s.genre, l);

            if (score > worstScore)
            {
                worstScore = score;
                worst = idx;
            }
        }

        // Nothing left that we are allowed to thin.
        if (worst < 0)
            break;

        auto& p = kit.patterns[(size_t) worst];

        int quietest = -1;
        float lowest = 2.0f;

        for (int i = 0; i < p.numSteps; ++i)
            if (p.steps[(size_t) i].on && p.steps[(size_t) i].velocity < lowest)
            {
                lowest = p.steps[(size_t) i].velocity;
                quietest = i;
            }

        if (quietest < 0)
            break;

        p.steps[(size_t) quietest].clear();
    }
}

// ============================================================================
//  entry points
// ============================================================================

/** Build one lane, without touching the shared budget.

    The budget is a property of the finished kit, so charging it per lane during
    a full generate makes the lanes generated last pay for every lane before
    them - and Percussion is last in the order, which is exactly how it ended up
    thinned to a fill while the hi-hat kept twenty hits. */
static void buildLane (Kit& kit, LaneId lane, const GenSettings& s, int laneSeed)
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
    applyBarVariation (kit.patterns[(size_t) idx], lane, s, rng,
                       laneImportance (s.genre, lane) >= 1.0f);
    applyFill (kit, lane, s, rng);
    applyGroove (kit.patterns[(size_t) idx], lane, s, rng,
                 kit.lanes[(size_t) idx].dynamics);
    applyGenreGuards (kit.patterns[(size_t) idx], lane, s);
}

void Generator::generateLane (Kit& kit, LaneId lane, const GenSettings& s, int laneSeed)
{
    buildLane (kit, lane, s, laneSeed);

    // A single-lane reroll trims only itself - it must not quietly rewrite the
    // lanes the user just decided to keep.
    applyEnergyBudget (kit, s, (int) lane);
}

void Generator::generate (Kit& kit, const GenSettings& s)
{
    // Dependency order: everything downstream reads the kick, the hats read the
    // clap. Changing this order changes the groove, not just the code.
    static const LaneId order[] =
    {
        LaneId::Kick, LaneId::Clap, LaneId::Tom,
        LaneId::OpenHat, LaneId::ClosedHat,
        LaneId::Shaker, LaneId::Percussion, LaneId::Percussion2
    };

    static_assert ((int) LaneId::NumLanes == 8,
                   "add the new lane to the generation order - order is the groove");

    kit.setBars (s.bars);

    for (auto lane : order)
    {
        const int idx = (int) lane;

        if (! kit.lanes[(size_t) idx].enabled || kit.lanes[(size_t) idx].locked)
            continue;

        buildLane (kit, lane, s, s.seed * 7919 + idx * 104729);
    }

    applyEnergyBudget (kit, s);
}

} // namespace drumid
