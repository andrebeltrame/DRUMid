#pragma once

#include "../Engine/Types.h"
#include <juce_gui_basics/juce_gui_basics.h>

namespace drumid::ui
{

/** Lane glyphs, drawn as vectors so they stay crisp at any editor size.

    Each one is a shorthand for the instrument rather than a literal drawing:
    a beater circle for the kick, a burst for the clap, closed and open cymbals
    for the hats, a body with grains for the shaker.
*/
namespace Icons
{
    void drawLane (juce::Graphics&, LaneId, juce::Rectangle<float> area, juce::Colour);

    /** Die face, for the GENERATE button. */
    void drawDice (juce::Graphics&, juce::Rectangle<float> area, juce::Colour);

    /** Four-point sparkle, for SURPRISE. */
    void drawSparkle (juce::Graphics&, juce::Rectangle<float> area, juce::Colour);
}

} // namespace drumid::ui
