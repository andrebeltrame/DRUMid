#include "PatternLibrary.h"

#include <cstring>

namespace drumid
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
//    - afro house    : the percussion IS the track - clave and tresillo congas,
//                      a rolling shaker, a ghosted 16th ahead of the downbeat
//    - indie dance   : played-not-programmed - 8th hats with real shuffle,
//                      ghost snares, offbeat tambourine, broken kicks
//    - melodic techno: straight and patient - offbeat hat, backbeat with a long
//                      tail, sparse metallic ticks placed 3-against-4
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

    // ---------------------------------------------------------------- AFRO HOUSE
    { Genre::AfroHouse,    LaneId::Kick,      "four on the floor", "X...X...X...X...",                 0.0f, 1.0f, 3.0f },
    { Genre::AfroHouse,    LaneId::Kick,      "ghost pickup",      "X...X...X...X..o",                 0.2f, 1.0f, 2.0f },
    { Genre::AfroHouse,    LaneId::Kick,      "breathing",         "X..oX...X..oX...",                 0.4f, 1.0f, 1.5f },

    { Genre::AfroHouse,    LaneId::Clap,      "backbeat",          "....X.......X...",                 0.0f, 1.0f, 2.0f },
    { Genre::AfroHouse,    LaneId::Clap,      "only on 4",         "............X...",                 0.0f, 0.7f, 2.0f },
    { Genre::AfroHouse,    LaneId::Clap,      "ghost 2, hit 4",    "....o.......X...",                 0.2f, 1.0f, 1.5f },

    { Genre::AfroHouse,    LaneId::ClosedHat, "offbeat 8ths",      "..x...x...x...x.",                 0.0f, 0.7f, 2.0f },
    { Genre::AfroHouse,    LaneId::ClosedHat, "16ths",             "oxoxoxoxoxoxoxox",                 0.3f, 1.0f, 2.0f },
    { Genre::AfroHouse,    LaneId::ClosedHat, "hand 16ths",        "oXooxXoooXooxXoo",                 0.4f, 1.0f, 2.0f },

    { Genre::AfroHouse,    LaneId::OpenHat,   "offbeat",           "..X...X...X...X.",                 0.0f, 1.0f, 2.0f },
    { Genre::AfroHouse,    LaneId::OpenHat,   "and of 2 and 4",    "......X.......X.",                 0.0f, 0.7f, 1.5f },

    { Genre::AfroHouse,    LaneId::Shaker,    "son clave 3-2",     "X..X..X...X.X...",                 0.0f, 1.0f, 2.5f },
    { Genre::AfroHouse,    LaneId::Shaker,    "tresillo",          "X..x..X.x..X..x.",                 0.0f, 1.0f, 2.5f },
    { Genre::AfroHouse,    LaneId::Shaker,    "E(7,16)",           "x..x.x.x..x.x.x.",                 0.2f, 1.0f, 2.0f },
    { Genre::AfroHouse,    LaneId::Shaker,    "rolling conga",     "X.xX.xX.xX.xX.x.",                 0.4f, 1.0f, 2.0f },
    { Genre::AfroHouse,    LaneId::Shaker,    "16th shaker",       "oxoXoxoXoxoXoxoX",                 0.5f, 1.0f, 2.0f },

    // ---------------------------------------------------------------- INDIE DANCE
    { Genre::IndieDance,   LaneId::Kick,      "four on the floor", "X...X...X...X...",                 0.0f, 1.0f, 3.0f },
    { Genre::IndieDance,   LaneId::Kick,      "broken",            "X...X.....X.X...",                 0.4f, 1.0f, 1.8f },
    { Genre::IndieDance,   LaneId::Kick,      "bar-end push",      "X...X...X...X..x",                 0.3f, 1.0f, 1.5f },

    { Genre::IndieDance,   LaneId::Clap,      "backbeat",          "....X.......X...",                 0.0f, 1.0f, 3.0f },
    { Genre::IndieDance,   LaneId::Clap,      "backbeat + ghosts", "....X...o...X..o",                 0.3f, 1.0f, 2.0f },
    { Genre::IndieDance,   LaneId::Clap,      "played backbeat",   "....X..o....X.o.",                 0.4f, 1.0f, 1.5f },

    { Genre::IndieDance,   LaneId::ClosedHat, "8ths",              "x.x.x.x.x.x.x.x.",                 0.0f, 1.0f, 2.5f },
    { Genre::IndieDance,   LaneId::ClosedHat, "16ths",             "xoxoxoxoxoxoxoxo",                 0.4f, 1.0f, 2.0f },
    { Genre::IndieDance,   LaneId::ClosedHat, "offbeat 8ths",      "..x...x...x...x.",                 0.0f, 0.6f, 1.5f },

    { Genre::IndieDance,   LaneId::OpenHat,   "offbeat",           "..X...X...X...X.",                 0.0f, 1.0f, 2.5f },
    { Genre::IndieDance,   LaneId::OpenHat,   "and of 2 and 4",    "......X.......X.",                 0.0f, 0.7f, 1.5f },

    { Genre::IndieDance,   LaneId::Shaker,    "offbeat tambourine","..x...x...x...x.",                 0.0f, 1.0f, 2.5f },
    { Genre::IndieDance,   LaneId::Shaker,    "16ths",             "oxoxoxoxoxoxoxox",                 0.3f, 1.0f, 2.0f },
    { Genre::IndieDance,   LaneId::Shaker,    "son clave 3-2",     "X..X..X...X.X...",                 0.4f, 1.0f, 1.2f },

    // ---------------------------------------------------------------- MELODIC TECHNO
    { Genre::MelodicTechno, LaneId::Kick,      "straight four",    "X...X...X...X...",                 0.0f, 1.0f, 4.0f },
    { Genre::MelodicTechno, LaneId::Kick,      "ghost pickup",     "X...X...X...X..o",                 0.4f, 1.0f, 1.8f },

    { Genre::MelodicTechno, LaneId::Clap,      "backbeat",         "....X.......X...",                 0.0f, 1.0f, 3.0f },
    { Genre::MelodicTechno, LaneId::Clap,      "on 3 only",        "........X.......",                 0.0f, 0.5f, 1.5f },
    { Genre::MelodicTechno, LaneId::Clap,      "backbeat + ghost", "....X.......X..o",                 0.5f, 1.0f, 1.2f },

    { Genre::MelodicTechno, LaneId::ClosedHat, "offbeat 8ths",    "..X...X...X...X.",                 0.0f, 1.0f, 2.5f },
    { Genre::MelodicTechno, LaneId::ClosedHat, "16ths offbeat acc","xoXoxoXoxoXoxoXo",                 0.4f, 1.0f, 2.5f },
    { Genre::MelodicTechno, LaneId::ClosedHat, "16ths accent 4",   "XoooxoooXoooxooo",                 0.5f, 1.0f, 1.5f },

    { Genre::MelodicTechno, LaneId::OpenHat,   "offbeat",         "..X...X...X...X.",                 0.2f, 1.0f, 2.0f },
    { Genre::MelodicTechno, LaneId::OpenHat,   "bar lift",        "..............X.",                 0.0f, 0.8f, 2.0f },
    { Genre::MelodicTechno, LaneId::OpenHat,   "and of 2 and 4",  "......X.......X.",                 0.0f, 0.7f, 1.5f },

    { Genre::MelodicTechno, LaneId::Shaker,    "sparse metal",    "...x......x.....",                 0.0f, 0.8f, 2.0f },
    { Genre::MelodicTechno, LaneId::Shaker,    "3-against-4",     "x..x..x..x..x...",                 0.2f, 1.0f, 2.0f },
    { Genre::MelodicTechno, LaneId::Shaker,    "tick pairs",      "..x.x.....x.x...",                 0.3f, 1.0f, 1.8f },
    { Genre::MelodicTechno, LaneId::Shaker,    "16ths",           "oxoxoxoxoxoxoxox",                 0.7f, 1.0f, 1.2f },

    // ============================================================================
    //  TOM
    //
    //  The tom is a colour, not a timekeeper - one or two placed hits per bar in
    //  most of these styles. Techno is the exception, where the rolling 8th tom
    //  is a groove in its own right, and afro house, where it answers the conga.
    // ============================================================================
    { Genre::OrganicHouse,  LaneId::Tom, "answer on the and", "......x.........",                 0.0f, 1.0f, 2.5f },
    { Genre::OrganicHouse,  LaneId::Tom, "3-against-4",       "x.....x.....x...",                 0.3f, 1.0f, 2.0f },
    { Genre::OrganicHouse,  LaneId::Tom, "tribal pair",       "..x.......x.....",                 0.2f, 1.0f, 2.0f },
    { Genre::OrganicHouse,  LaneId::Tom, "bar tail",          "..............x.",                 0.0f, 0.7f, 1.5f },

    { Genre::AfroHouse,     LaneId::Tom, "djembe answer",     "..x..x....x.....",                 0.0f, 1.0f, 2.5f },
    { Genre::AfroHouse,     LaneId::Tom, "tribal roll",       "x.xx..x...x.x...",                 0.4f, 1.0f, 2.0f },
    { Genre::AfroHouse,     LaneId::Tom, "tresillo low",      "x..x..x.........",                 0.2f, 1.0f, 2.0f },
    { Genre::AfroHouse,     LaneId::Tom, "sparse accent",     "......x.........",                 0.0f, 0.6f, 1.5f },

    { Genre::IndieDance,    LaneId::Tom, "floor tom offbeat", "......x.......x.",                 0.0f, 1.0f, 2.5f },
    { Genre::IndieDance,    LaneId::Tom, "live fill",         "....x..x....x..x",                 0.4f, 1.0f, 2.0f },
    { Genre::IndieDance,    LaneId::Tom, "single accent",     "..........x.....",                 0.0f, 0.7f, 1.5f },

    { Genre::MelodicHouse,  LaneId::Tom, "one per bar",       "..........x.....",                 0.0f, 0.8f, 2.5f },
    { Genre::MelodicHouse,  LaneId::Tom, "bar tail",          "..............x.",                 0.0f, 1.0f, 2.0f },
    { Genre::MelodicHouse,  LaneId::Tom, "answer pair",       "......x...x.....",                 0.4f, 1.0f, 1.5f },

    { Genre::MelodicTechno, LaneId::Tom, "deep single",       "........x.......",                 0.0f, 1.0f, 2.5f },
    { Genre::MelodicTechno, LaneId::Tom, "half-bar pulse",    "x.......x.......",                 0.3f, 1.0f, 2.0f },
    { Genre::MelodicTechno, LaneId::Tom, "bar tail",          "..............x.",                 0.0f, 0.8f, 1.5f },

    { Genre::Techno,        LaneId::Tom, "rolling 8ths",      "x.x.x.x.x.x.x.x.",                 0.5f, 1.0f, 2.0f },
    { Genre::Techno,        LaneId::Tom, "backbeat answer",   "....x.......x...",                 0.0f, 1.0f, 2.0f },
    { Genre::Techno,        LaneId::Tom, "sparse hit",        "..........x.....",                 0.0f, 0.7f, 2.0f },

    // ============================================================================
    //  PERCUSSION  (congas, bongos, rim, metal)
    //
    //  In organic and afro house this lane is the lead voice, so it gets the full
    //  clave and tumbao vocabulary. In the techno family it is one or two
    //  metallic ticks placed against the grid.
    // ============================================================================
    { Genre::OrganicHouse,  LaneId::Percussion, "tumbao",          "..x.x..x..x.x..x",         0.3f, 1.0f, 2.5f },
    { Genre::OrganicHouse,  LaneId::Percussion, "son clave 3-2",   "X..X..X...X.X...",         0.0f, 1.0f, 2.5f },
    { Genre::OrganicHouse,  LaneId::Percussion, "rumba clave 3-2", "X..X...X..X.X...",         0.0f, 1.0f, 2.2f },
    { Genre::OrganicHouse,  LaneId::Percussion, "tresillo",        "X..x..X.........",         0.0f, 0.8f, 2.0f },
    { Genre::OrganicHouse,  LaneId::Percussion, "conga answer",    "...x..x...x..x..",         0.2f, 1.0f, 2.0f },

    { Genre::AfroHouse,     LaneId::Percussion, "rolling conga",   "X.xX.xX.xX.xX.x.",         0.3f, 1.0f, 2.5f },
    { Genre::AfroHouse,     LaneId::Percussion, "tumbao",          "..x.x..x..x.x..x",         0.2f, 1.0f, 2.5f },
    { Genre::AfroHouse,     LaneId::Percussion, "son clave 3-2",   "X..X..X...X.X...",         0.0f, 1.0f, 2.2f },
    { Genre::AfroHouse,     LaneId::Percussion, "E(7,16) bongo",   "x..x.x.x..x.x.x.",         0.2f, 1.0f, 2.2f },
    { Genre::AfroHouse,     LaneId::Percussion, "cascara",         "x.xx.x.xx.xx.x.x",         0.5f, 1.0f, 1.8f },

    { Genre::IndieDance,    LaneId::Percussion, "cowbell offbeat", "..x...x...x...x.",         0.0f, 1.0f, 2.5f },
    { Genre::IndieDance,    LaneId::Percussion, "rim answer",      "....x.......x..x",         0.3f, 1.0f, 2.0f },
    { Genre::IndieDance,    LaneId::Percussion, "clave",           "X..X..X...X.X...",         0.2f, 1.0f, 1.8f },

    { Genre::MelodicHouse,  LaneId::Percussion, "offbeat conga",   "..x...x...x...x.",         0.0f, 1.0f, 2.2f },
    { Genre::MelodicHouse,  LaneId::Percussion, "16th answer",     "...x..x......x..",         0.3f, 1.0f, 2.0f },
    { Genre::MelodicHouse,  LaneId::Percussion, "tresillo",        "X..x..x.........",         0.0f, 0.8f, 1.8f },

    { Genre::MelodicTechno, LaneId::Percussion, "metal tick",      "...x......x.....",         0.0f, 1.0f, 2.5f },
    { Genre::MelodicTechno, LaneId::Percussion, "3-against-4",     "x..x..x..x..x...",         0.3f, 1.0f, 2.0f },
    { Genre::MelodicTechno, LaneId::Percussion, "tick pairs",      "..x.x.....x.x...",         0.3f, 1.0f, 1.8f },

    { Genre::Techno,        LaneId::Percussion, "sparse metal",    "......x.........",         0.0f, 1.0f, 2.5f },
    { Genre::Techno,        LaneId::Percussion, "syncopated pair", "...x........x...",         0.0f, 1.0f, 2.2f },
    { Genre::Techno,        LaneId::Percussion, "3-against-4",     "x..x..x..x..x...",         0.4f, 1.0f, 1.8f },

    // ============================================================================
    //  PERCUSSION 2  (optional second hand voice)
    //
    //  This lane answers the first percussion rather than doubling it, so the
    //  seeds are deliberately sparser and sit off the clave's strong points. The
    //  generator also displaces it away from whatever the shaker and the first
    //  percussion already play - two hand voices on the same rhythm is one
    //  voice's worth of groove for two voices' worth of mud.
    // ============================================================================
    { Genre::OrganicHouse,  LaneId::Percussion2, "offbeat answer",  "...x..x...x..x..",         0.0f, 1.0f, 2.5f },
    { Genre::OrganicHouse,  LaneId::Percussion2, "tick pairs",      "..x.x.....x.x...",         0.0f, 1.0f, 2.2f },
    { Genre::OrganicHouse,  LaneId::Percussion2, "sparse call",     "....x.......x..x",         0.0f, 0.8f, 2.0f },
    { Genre::OrganicHouse,  LaneId::Percussion2, "2-side clave",    "......X...X.....",         0.2f, 1.0f, 1.8f },

    { Genre::AfroHouse,     LaneId::Percussion2, "high answer",     "...x..x...x..x..",         0.0f, 1.0f, 2.5f },
    { Genre::AfroHouse,     LaneId::Percussion2, "bongo fill",      "...x.x....x.x.x.",         0.3f, 1.0f, 2.2f },
    { Genre::AfroHouse,     LaneId::Percussion2, "2-side clave",    "......X...X.....",         0.0f, 1.0f, 2.0f },
    { Genre::AfroHouse,     LaneId::Percussion2, "sparse call",     "..x....x..x....x",         0.0f, 0.9f, 2.0f },

    { Genre::IndieDance,    LaneId::Percussion2, "backbeat answer", "....x.......x...",         0.0f, 1.0f, 2.2f },
    { Genre::IndieDance,    LaneId::Percussion2, "offbeat tick",    "..x...x.........",         0.0f, 1.0f, 2.0f },
    { Genre::IndieDance,    LaneId::Percussion2, "late call",       "..............x.",         0.0f, 0.7f, 1.5f },

    { Genre::MelodicHouse,  LaneId::Percussion2, "offbeat tick",    "......x.......x.",         0.0f, 1.0f, 2.2f },
    { Genre::MelodicHouse,  LaneId::Percussion2, "16th answer",     "...x......x.....",         0.0f, 1.0f, 2.0f },
    { Genre::MelodicHouse,  LaneId::Percussion2, "one per bar",     "..........x.....",         0.0f, 0.7f, 1.6f },

    { Genre::MelodicTechno, LaneId::Percussion2, "far tick",        ".....x.....x....",         0.0f, 1.0f, 2.2f },
    { Genre::MelodicTechno, LaneId::Percussion2, "syncopated pair", "...x........x...",         0.0f, 1.0f, 2.0f },
    { Genre::MelodicTechno, LaneId::Percussion2, "one per bar",     "........x.......",         0.0f, 0.8f, 1.6f },

    { Genre::Techno,        LaneId::Percussion2, "single hit",      "........x.......",         0.0f, 1.0f, 2.2f },
    { Genre::Techno,        LaneId::Percussion2, "syncopated pair", "..x.........x...",         0.0f, 1.0f, 2.0f },
    { Genre::Techno,        LaneId::Percussion2, "3-against-4",     "..x..x..x..x....",         0.4f, 1.0f, 1.6f },

    // ============================================================================
    //  CINEMATIC
    //
    //  Half-time, and the only style here that is not four-on-the-floor: the
    //  kick lands on 1 and 3 and the big backbeat lands on 3. The toms are the
    //  lead voice, hats are nearly absent, and the space between the hits is
    //  the point - so the seeds are sparse on purpose and the tom carries the
    //  weight the hi-hat carries everywhere else.
    // ============================================================================
    { Genre::Cinematic, LaneId::Kick,       "half-time",        "X.......X.......",         0.0f, 1.0f, 3.5f },
    { Genre::Cinematic, LaneId::Kick,       "with pickup",      "X.......X.....o.",         0.3f, 1.0f, 1.8f },
    { Genre::Cinematic, LaneId::Kick,       "three hits",       "X.......X...X...",         0.5f, 1.0f, 1.5f },
    { Genre::Cinematic, LaneId::Kick,       "one per bar",      "X...............",         0.0f, 0.4f, 1.5f },

    { Genre::Cinematic, LaneId::Clap,       "big hit on 3",     "........X.......",         0.0f, 1.0f, 3.0f },
    { Genre::Cinematic, LaneId::Clap,       "hit 3 + tail",     "........X.....o.",         0.3f, 1.0f, 1.5f },
    { Genre::Cinematic, LaneId::Clap,       "double-time",      "....X.......X...",         0.6f, 1.0f, 1.2f },

    { Genre::Cinematic, LaneId::Tom,        "tresillo taiko",   "X..X..X.........",         0.0f, 1.0f, 3.0f },
    { Genre::Cinematic, LaneId::Tom,        "answer",           "X...X...X..X....",         0.3f, 1.0f, 2.5f },
    { Genre::Cinematic, LaneId::Tom,        "rolling build",    "x.x.x.x.x.x.x.x.",         0.6f, 1.0f, 2.0f },
    { Genre::Cinematic, LaneId::Tom,        "two heavy",        "X.....X.X.......",         0.0f, 0.8f, 2.0f },

    { Genre::Cinematic, LaneId::ClosedHat,  "one tick",         "........x.......",         0.0f, 0.6f, 2.0f },
    { Genre::Cinematic, LaneId::ClosedHat,  "sparse pulse",     "..x.....x.....x.",         0.2f, 1.0f, 1.8f },
    { Genre::Cinematic, LaneId::ClosedHat,  "tension 8ths",     "x.x.x.x.x.x.x.x.",         0.8f, 1.0f, 1.2f },

    { Genre::Cinematic, LaneId::OpenHat,    "bar tail",         "..............X.",         0.0f, 1.0f, 2.5f },
    { Genre::Cinematic, LaneId::OpenHat,    "on 3",             "........X.......",         0.0f, 0.7f, 1.5f },

    { Genre::Cinematic, LaneId::Shaker,     "sparse",           "...x........x...",         0.0f, 1.0f, 2.0f },
    { Genre::Cinematic, LaneId::Shaker,     "quarters",         "x...x...x...x...",         0.4f, 1.0f, 1.5f },

    { Genre::Cinematic, LaneId::Percussion, "single impact",    "X...............",         0.0f, 0.7f, 2.5f },
    { Genre::Cinematic, LaneId::Percussion, "two impacts",      "X.......X.......",         0.2f, 1.0f, 2.2f },
    { Genre::Cinematic, LaneId::Percussion, "offbeat metal",    "....X.......X...",         0.4f, 1.0f, 1.8f },

    { Genre::Cinematic, LaneId::Percussion2,"late answer",      "..............X.",         0.0f, 1.0f, 2.2f },
    { Genre::Cinematic, LaneId::Percussion2,"mid answer",       "......X.........",         0.0f, 1.0f, 2.0f },

    // ============================================================================
    //  PROGRESSIVE HOUSE
    //
    //  The wide, driving end of the house family: the offbeat open hat is the
    //  engine, 16th hats sit under it, and the percussion rolls rather than
    //  syncopates. Straighter and more relentless than melodic house.
    // ============================================================================
    { Genre::ProgressiveHouse, LaneId::Kick,       "four on the floor", "X...X...X...X...",                 0.0f, 1.0f, 3.5f },
    { Genre::ProgressiveHouse, LaneId::Kick,       "ghost pickup",      "X...X...X...X..o",                 0.3f, 1.0f, 1.8f },
    { Genre::ProgressiveHouse, LaneId::Kick,       "bar-end lift",      "X...X...X...X...X...X...X...X..x", 0.4f, 1.0f, 1.2f },

    { Genre::ProgressiveHouse, LaneId::Clap,       "backbeat",          "....X.......X...",                 0.0f, 1.0f, 3.0f },
    { Genre::ProgressiveHouse, LaneId::Clap,       "backbeat + ghost",  "....X...o...X...",                 0.4f, 1.0f, 1.8f },
    { Genre::ProgressiveHouse, LaneId::Clap,       "push on 4",         "....X.......X..o",                 0.5f, 1.0f, 1.2f },

    { Genre::ProgressiveHouse, LaneId::ClosedHat,  "offbeat 8ths",      "..x...x...x...x.",                 0.0f, 0.6f, 2.2f },
    { Genre::ProgressiveHouse, LaneId::ClosedHat,  "16ths offbeat acc", "xoXoxoXoxoXoxoXo",                 0.3f, 1.0f, 2.5f },
    { Genre::ProgressiveHouse, LaneId::ClosedHat,  "driving 16ths",     "oxoxoxoxoxoxoxox",                 0.4f, 1.0f, 2.2f },

    { Genre::ProgressiveHouse, LaneId::OpenHat,    "offbeat",           "..X...X...X...X.",                 0.0f, 1.0f, 3.5f },
    { Genre::ProgressiveHouse, LaneId::OpenHat,    "and of 2 and 4",    "......X.......X.",                 0.0f, 0.6f, 1.5f },

    { Genre::ProgressiveHouse, LaneId::Tom,        "one per bar",       "..........x.....",                 0.0f, 0.8f, 2.0f },
    { Genre::ProgressiveHouse, LaneId::Tom,        "answer pair",       "......x.......x.",                 0.3f, 1.0f, 1.8f },
    { Genre::ProgressiveHouse, LaneId::Tom,        "rolling build",     "x.x.x.x.x.x.x.x.",                 0.7f, 1.0f, 1.2f },

    { Genre::ProgressiveHouse, LaneId::Shaker,     "16ths",             "oxoxoxoxoxoxoxox",                 0.2f, 1.0f, 2.5f },
    { Genre::ProgressiveHouse, LaneId::Shaker,     "offbeat 16ths",     ".x.x.x.x.x.x.x.x",                 0.0f, 0.8f, 2.0f },
    { Genre::ProgressiveHouse, LaneId::Shaker,     "offbeat 8ths",      "..x...x...x...x.",                 0.0f, 0.7f, 1.8f },

    { Genre::ProgressiveHouse, LaneId::Percussion, "offbeat conga",     "..x...x...x...x.",                 0.0f, 1.0f, 2.2f },
    { Genre::ProgressiveHouse, LaneId::Percussion, "rolling 16ths",     "x..x..x...x.x...",                 0.3f, 1.0f, 2.0f },
    { Genre::ProgressiveHouse, LaneId::Percussion, "sparse tick",       "...x......x.....",                 0.0f, 0.8f, 1.8f },

    { Genre::ProgressiveHouse, LaneId::Percussion2,"far answer",        "......x.......x.",                 0.0f, 1.0f, 2.2f },
    { Genre::ProgressiveHouse, LaneId::Percussion2,"backbeat tick",     "....x.......x...",                 0.0f, 1.0f, 2.0f },

    // ============================================================================
    //  BIG ROOM EDM
    //
    //  Festival main stage: everything serves the drop. Hats are the offbeat and
    //  the 16th burst, the clap is layered and loud, and the tom exists to roll
    //  into the next section. Percussion stays out of the way on purpose - there
    //  is no room for a conga under a lead that wide.
    // ============================================================================
    { Genre::BigRoomEDM, LaneId::Kick,       "four on the floor", "X...X...X...X...",         0.0f, 1.0f, 4.0f },
    { Genre::BigRoomEDM, LaneId::Kick,       "push into the bar", "X...X...X...X.x.",         0.5f, 1.0f, 1.5f },

    { Genre::BigRoomEDM, LaneId::Clap,       "backbeat",          "....X.......X...",         0.0f, 1.0f, 3.5f },
    { Genre::BigRoomEDM, LaneId::Clap,       "backbeat + roll",   "....X.......X.:.",         0.5f, 1.0f, 1.5f },

    { Genre::BigRoomEDM, LaneId::ClosedHat,  "offbeat 8ths",      "..X...X...X...X.",         0.0f, 1.0f, 3.0f },
    { Genre::BigRoomEDM, LaneId::ClosedHat,  "16ths accent 4",    "XoooxoooXoooxooo",         0.4f, 1.0f, 2.0f },
    { Genre::BigRoomEDM, LaneId::ClosedHat,  "16th burst",        "xxxxxxxxxxxxxxxx",         0.7f, 1.0f, 1.5f },

    { Genre::BigRoomEDM, LaneId::OpenHat,    "offbeat",           "..X...X...X...X.",         0.0f, 1.0f, 3.0f },
    { Genre::BigRoomEDM, LaneId::OpenHat,    "bar lift",          "..............X.",         0.0f, 0.7f, 1.8f },

    { Genre::BigRoomEDM, LaneId::Tom,        "roll into the bar", "............x.x.",         0.0f, 1.0f, 2.5f },
    { Genre::BigRoomEDM, LaneId::Tom,        "build roll",        "x.x.x.x.x.x.x.x.",         0.6f, 1.0f, 2.0f },
    { Genre::BigRoomEDM, LaneId::Tom,        "single accent",     "..........x.....",         0.0f, 0.7f, 1.5f },

    { Genre::BigRoomEDM, LaneId::Shaker,     "offbeat",           "..x...x...x...x.",         0.0f, 1.0f, 2.2f },
    { Genre::BigRoomEDM, LaneId::Shaker,     "16ths",             "oxoxoxoxoxoxoxox",         0.4f, 1.0f, 1.8f },

    { Genre::BigRoomEDM, LaneId::Percussion, "sparse tick",       "...x......x.....",         0.0f, 1.0f, 2.2f },
    { Genre::BigRoomEDM, LaneId::Percussion, "syncopated pair",   "......x.......x.",         0.0f, 1.0f, 2.0f },

    { Genre::BigRoomEDM, LaneId::Percussion2,"late tick",         "..............x.",         0.0f, 1.0f, 2.2f },
    { Genre::BigRoomEDM, LaneId::Percussion2,"mid tick",          "........x.......",         0.0f, 1.0f, 2.0f },

    // ============================================================================
    //  TOM and OPEN HAT - the answering voices
    //
    //  These two are where a kit stops sounding like a loop, so they get the
    //  widest bank and several two-bar seeds. A 32-step grid does not tile: the
    //  second bar is written to differ from the first, which is the cheapest
    //  possible cure for the repetition that one-bar seeds cause everywhere.
    // ============================================================================

    // ---- Cinematic
    { Genre::Cinematic, LaneId::Tom, "call and answer",   "X..X..X.........X..X..X...X.X...", 0.0f, 1.0f, 2.5f },
    { Genre::Cinematic, LaneId::Tom, "gathering",         "X.......X.......X...X...X.X.X...", 0.3f, 1.0f, 2.2f },
    { Genre::Cinematic, LaneId::Tom, "triplet feel",      "X..X..X..X..X...", 0.2f, 1.0f, 2.0f },
    { Genre::Cinematic, LaneId::Tom, "far apart",         "X...........X...", 0.0f, 0.6f, 1.8f },
    { Genre::Cinematic, LaneId::Tom, "run into the bar",  "X.......X.....x.X.......x.xxXXXX", 0.5f, 1.0f, 1.8f },

    { Genre::Cinematic, LaneId::OpenHat, "swell then rest", "..............X.................", 0.0f, 1.0f, 2.0f },
    { Genre::Cinematic, LaneId::OpenHat, "answer on 3",     "........X...............X.......", 0.0f, 1.0f, 1.8f },
    { Genre::Cinematic, LaneId::OpenHat, "two swells",      "......X.........X...............", 0.3f, 1.0f, 1.5f },

    // ---- Organic house
    { Genre::OrganicHouse, LaneId::Tom, "call and answer", "......x...........x...x.........", 0.0f, 1.0f, 2.2f },
    { Genre::OrganicHouse, LaneId::Tom, "walking",         "..x.......x.....x.....x.........", 0.3f, 1.0f, 2.0f },
    { Genre::OrganicHouse, LaneId::Tom, "tresillo pair",   "x..x..x.........", 0.2f, 1.0f, 2.0f },
    { Genre::OrganicHouse, LaneId::Tom, "late push",       "..............x.", 0.0f, 0.8f, 1.8f },

    { Genre::OrganicHouse, LaneId::OpenHat, "alternating bars", "......X.................X.......", 0.0f, 1.0f, 2.2f },
    { Genre::OrganicHouse, LaneId::OpenHat, "three then one",   "..X...X...X.......X.............", 0.3f, 1.0f, 2.0f },
    { Genre::OrganicHouse, LaneId::OpenHat, "on the a",         "...X.......X....", 0.2f, 1.0f, 1.8f },
    { Genre::OrganicHouse, LaneId::OpenHat, "long short",       "..X.......X...X.", 0.4f, 1.0f, 1.6f },

    // ---- Afro house
    { Genre::AfroHouse, LaneId::Tom, "djembe answer",   "..x..x....x.......x..x....x.x...", 0.0f, 1.0f, 2.2f },
    { Genre::AfroHouse, LaneId::Tom, "climbing",        "x.....x.....x.....x...x...x.x...", 0.4f, 1.0f, 2.0f },
    { Genre::AfroHouse, LaneId::Tom, "off the clave",   "...x..x...x.....", 0.2f, 1.0f, 2.0f },
    { Genre::AfroHouse, LaneId::Tom, "two low hits",    "x.......x.......", 0.0f, 0.7f, 1.8f },

    { Genre::AfroHouse, LaneId::OpenHat, "call and rest",  "..X...X.................X.......", 0.0f, 1.0f, 2.2f },
    { Genre::AfroHouse, LaneId::OpenHat, "on the a",       "...X.......X....", 0.2f, 1.0f, 2.0f },
    { Genre::AfroHouse, LaneId::OpenHat, "pairs",          "..X.X.....X.X...", 0.4f, 1.0f, 1.8f },
    { Genre::AfroHouse, LaneId::OpenHat, "every other bar","..X...X...X...X.................", 0.3f, 1.0f, 1.8f },

    // ---- Indie dance
    { Genre::IndieDance, LaneId::Tom, "live fill",      "....x..x............x..x....x.x.", 0.0f, 1.0f, 2.2f },
    { Genre::IndieDance, LaneId::Tom, "floor pulse",    "......x.......x.", 0.0f, 1.0f, 2.0f },
    { Genre::IndieDance, LaneId::Tom, "answer the snare","............x..x", 0.3f, 1.0f, 1.8f },
    { Genre::IndieDance, LaneId::Tom, "roll out",       "................x.x.x.x.x.x.xxx.", 0.5f, 1.0f, 1.6f },

    { Genre::IndieDance, LaneId::OpenHat, "offbeat then rest", "..X...X...X...X.......X.........", 0.0f, 1.0f, 2.2f },
    { Genre::IndieDance, LaneId::OpenHat, "on the a",          "...X.......X....", 0.2f, 1.0f, 1.8f },
    { Genre::IndieDance, LaneId::OpenHat, "late pair",         "..........X...X.", 0.3f, 1.0f, 1.6f },

    // ---- Melodic house
    { Genre::MelodicHouse, LaneId::Tom, "answer",        "..........x...............x.x...", 0.0f, 1.0f, 2.2f },
    { Genre::MelodicHouse, LaneId::Tom, "two per phrase","......x.................x.......", 0.2f, 1.0f, 2.0f },
    { Genre::MelodicHouse, LaneId::Tom, "late push",     "..............x.", 0.0f, 1.0f, 1.8f },

    { Genre::MelodicHouse, LaneId::OpenHat, "offbeat then lift", "..X...X...X...X...X...X.......X.", 0.0f, 1.0f, 2.5f },
    { Genre::MelodicHouse, LaneId::OpenHat, "on the a",          "...X.......X....", 0.2f, 1.0f, 2.0f },
    { Genre::MelodicHouse, LaneId::OpenHat, "drop a bar",        "..X...X...X...X.................", 0.3f, 1.0f, 1.8f },
    { Genre::MelodicHouse, LaneId::OpenHat, "pairs",             "..X.X.....X.X...", 0.5f, 1.0f, 1.5f },

    // ---- Progressive house
    { Genre::ProgressiveHouse, LaneId::Tom, "answer",       "..........x...............x.x...", 0.0f, 1.0f, 2.2f },
    { Genre::ProgressiveHouse, LaneId::Tom, "build out",    "................x...x...x.x.x.x.", 0.5f, 1.0f, 2.0f },
    { Genre::ProgressiveHouse, LaneId::Tom, "syncopated",   ".....x.....x....", 0.3f, 1.0f, 1.8f },

    { Genre::ProgressiveHouse, LaneId::OpenHat, "offbeat then lift", "..X...X...X...X...X...X.......X.", 0.0f, 1.0f, 2.5f },
    { Genre::ProgressiveHouse, LaneId::OpenHat, "drop a bar",        "..X...X...X...X.................", 0.3f, 1.0f, 2.0f },
    { Genre::ProgressiveHouse, LaneId::OpenHat, "on the a",          "...X.......X....", 0.3f, 1.0f, 1.6f },

    // ---- Melodic techno
    { Genre::MelodicTechno, LaneId::Tom, "deep answer",   "........x.......................", 0.0f, 1.0f, 2.2f },
    { Genre::MelodicTechno, LaneId::Tom, "every other",   "x.......x.......................", 0.2f, 1.0f, 2.0f },
    { Genre::MelodicTechno, LaneId::Tom, "late run",      "..............................x.", 0.0f, 0.8f, 1.8f },
    { Genre::MelodicTechno, LaneId::Tom, "pulse pair",    "....x.......x...", 0.4f, 1.0f, 1.6f },

    { Genre::MelodicTechno, LaneId::OpenHat, "lift each phrase", "..............X.................", 0.0f, 1.0f, 2.2f },
    { Genre::MelodicTechno, LaneId::OpenHat, "offbeat then rest","..X...X...X...X.......X.........", 0.3f, 1.0f, 2.0f },
    { Genre::MelodicTechno, LaneId::OpenHat, "on the a",         "...X.......X....", 0.3f, 1.0f, 1.6f },

    // ---- Big room EDM
    { Genre::BigRoomEDM, LaneId::Tom, "roll then rest", "............x.x.................", 0.0f, 1.0f, 2.2f },
    { Genre::BigRoomEDM, LaneId::Tom, "build out",      "................x...x...x.x.xxxx", 0.5f, 1.0f, 2.0f },
    { Genre::BigRoomEDM, LaneId::Tom, "single accent",  "..........x.....", 0.0f, 1.0f, 1.6f },

    { Genre::BigRoomEDM, LaneId::OpenHat, "offbeat then lift", "..X...X...X...X...X...X.......X.", 0.0f, 1.0f, 2.2f },
    { Genre::BigRoomEDM, LaneId::OpenHat, "lift only",         "..............X.................", 0.0f, 1.0f, 2.0f },
    { Genre::BigRoomEDM, LaneId::OpenHat, "pairs",             "..X.X.....X.X...", 0.5f, 1.0f, 1.5f },

    // ---- Techno
    { Genre::Techno, LaneId::Tom, "rolling then rest", "x.x.x.x.x.x.x.x.................", 0.4f, 1.0f, 2.0f },
    { Genre::Techno, LaneId::Tom, "answer",            "....x.......x...................", 0.0f, 1.0f, 2.0f },
    { Genre::Techno, LaneId::Tom, "hypnotic pair",     "......x.......x.", 0.3f, 1.0f, 1.8f },
    { Genre::Techno, LaneId::Tom, "late run",          "..........................x.x.x.", 0.5f, 1.0f, 1.6f },

    { Genre::Techno, LaneId::OpenHat, "offbeat then rest", "..X...X...X...X.......X.........", 0.0f, 1.0f, 2.2f },
    { Genre::Techno, LaneId::OpenHat, "lift each phrase",  "..............X.................", 0.0f, 1.0f, 2.0f },
    { Genre::Techno, LaneId::OpenHat, "on the a",          "...X.......X....", 0.3f, 1.0f, 1.6f },
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

} // namespace drumid
