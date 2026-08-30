#include "Icons.h"

namespace drumid::ui
{

static void drawKick (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c)
{
    // Drum head seen face on, with the beater in the middle.
    const float line = juce::jmax (1.0f, r.getWidth() * 0.09f);

    g.setColour (c);
    g.drawEllipse (r.reduced (line * 0.5f), line);
    g.fillEllipse (r.reduced (r.getWidth() * 0.33f));
}

static void drawClap (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c)
{
    // A spark: rays radiating from the centre, longest on the diagonals so it
    // reads as an impact rather than an asterisk.
    const float line = juce::jmax (1.2f, r.getWidth() * 0.11f);
    const auto centre = r.getCentre();
    const float inner = r.getWidth() * 0.17f;

    g.setColour (c);

    for (int i = 0; i < 6; ++i)
    {
        const float angle = juce::MathConstants<float>::twoPi * (float) i / 6.0f
                          + juce::MathConstants<float>::pi * 0.16f;
        const float outer = r.getWidth() * ((i % 2 == 0) ? 0.48f : 0.36f);

        juce::Path p;
        p.startNewSubPath (centre.translated (std::cos (angle) * inner, std::sin (angle) * inner));
        p.lineTo          (centre.translated (std::cos (angle) * outer, std::sin (angle) * outer));

        g.strokePath (p, juce::PathStrokeType (line, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
    }
}

static void drawHat (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c, bool open)
{
    const float line = juce::jmax (1.0f, r.getWidth() * 0.09f);
    const float cy   = r.getCentreY();
    const float gap  = open ? r.getHeight() * 0.22f : r.getHeight() * 0.055f;
    const float w    = r.getWidth() * 0.92f;
    const float x    = r.getCentreX() - w * 0.5f;

    g.setColour (c);

    // Two cymbals, apart when open and all but touching when closed.
    g.fillRoundedRectangle (x, cy - gap - line, w, line * 1.3f, line * 0.6f);
    g.fillRoundedRectangle (x, cy + gap,        w, line * 1.3f, line * 0.6f);

    // Stand.
    g.fillRect (r.getCentreX() - line * 0.4f, cy + gap, line * 0.8f, r.getBottom() - (cy + gap));

    if (open)
    {
        // Two little ticks to say "ringing".
        g.setColour (c.withAlpha (0.6f));
        g.fillRoundedRectangle (x - line * 1.4f, cy - line * 0.4f, line * 0.9f, line * 0.8f, line * 0.4f);
        g.fillRoundedRectangle (x + w + line * 0.5f, cy - line * 0.4f, line * 0.9f, line * 0.8f, line * 0.4f);
    }
}

static void drawShaker (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c)
{
    // An egg shaker on a tilt, with motion lines so it reads as shaken rather
    // than as a plain oval.
    const float line = juce::jmax (1.2f, r.getWidth() * 0.095f);

    auto body = r.withSizeKeepingCentre (r.getWidth() * 0.42f, r.getHeight() * 0.68f)
                 .translated (-r.getWidth() * 0.10f, 0.0f);

    juce::Path egg;
    egg.addRoundedRectangle (body, body.getWidth() * 0.5f);
    egg.applyTransform (juce::AffineTransform::rotation (0.42f, r.getCentreX(), r.getCentreY()));

    g.setColour (c);
    g.fillPath (egg);

    for (int i = 0; i < 2; ++i)
    {
        const float x = r.getRight() - r.getWidth() * (0.20f - 0.13f * (float) i);
        const float h = r.getHeight() * (0.26f - 0.08f * (float) i);

        g.setColour (c.withAlpha (0.85f - 0.3f * (float) i));
        g.fillRoundedRectangle (x, r.getCentreY() - h * 0.5f, line, h, line * 0.5f);
    }
}

static void drawTom (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c)
{
    // A drum seen from the side: head on top, shell tapering down to the rim.
    const float line = juce::jmax (1.2f, r.getWidth() * 0.095f);

    auto body = r.reduced (r.getWidth() * 0.10f, r.getHeight() * 0.14f);
    const float headH = body.getHeight() * 0.34f;

    juce::Path shell;
    shell.startNewSubPath (body.getX(), body.getY() + headH * 0.5f);
    shell.lineTo (body.getX() + body.getWidth() * 0.11f, body.getBottom());
    shell.lineTo (body.getRight() - body.getWidth() * 0.11f, body.getBottom());
    shell.lineTo (body.getRight(), body.getY() + headH * 0.5f);

    g.setColour (c);
    g.strokePath (shell, juce::PathStrokeType (line, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    g.drawEllipse (body.getX(), body.getY(), body.getWidth(), headH, line);
}

static void drawPercussion (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c)
{
    // Bongos from above - two heads of different sizes, which reads instantly
    // as hand percussion and never gets confused with the tom.
    const float line = juce::jmax (1.2f, r.getWidth() * 0.095f);

    const float big = r.getWidth() * 0.58f;
    const float small = r.getWidth() * 0.44f;

    g.setColour (c);
    g.drawEllipse (r.getX(), r.getCentreY() - big * 0.5f, big, big, line);
    g.drawEllipse (r.getRight() - small, r.getCentreY() - small * 0.5f + r.getHeight() * 0.06f,
                   small, small, line);
}

static void drawCowbell (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c)
{
    // A cowbell: narrower at the top, flared at the mouth. Reads as "the other
    // hand percussion" without ever being mistaken for the bongos or the tom.
    const float line = juce::jmax (1.2f, r.getWidth() * 0.095f);

    auto b = r.reduced (r.getWidth() * 0.16f, r.getHeight() * 0.12f);
    const float topInset = b.getWidth() * 0.17f;

    juce::Path p;
    p.startNewSubPath (b.getX() + topInset, b.getY());
    p.lineTo (b.getRight() - topInset, b.getY());
    p.lineTo (b.getRight(), b.getBottom());
    p.lineTo (b.getX(), b.getBottom());
    p.closeSubPath();

    g.setColour (c);
    g.strokePath (p, juce::PathStrokeType (line, juce::PathStrokeType::mitered,
                                           juce::PathStrokeType::rounded));

    g.fillRect (b.getX() + topInset * 0.6f, b.getBottom() - line * 0.5f,
                b.getWidth() - topInset * 1.2f, line);
}

void Icons::drawLane (juce::Graphics& g, LaneId lane, juce::Rectangle<float> area, juce::Colour c)
{
    // Keep every glyph on the same square so the column reads as a column.
    auto r = area.withSizeKeepingCentre (juce::jmin (area.getWidth(), area.getHeight()),
                                         juce::jmin (area.getWidth(), area.getHeight()));

    switch (lane)
    {
        case LaneId::Kick:      drawKick   (g, r.reduced (r.getWidth() * 0.06f), c); break;
        case LaneId::Clap:      drawClap   (g, r.reduced (r.getWidth() * 0.06f), c); break;
        case LaneId::ClosedHat: drawHat    (g, r, c, false); break;
        case LaneId::OpenHat:   drawHat    (g, r, c, true);  break;
        case LaneId::Shaker:     drawShaker (g, r, c); break;
        case LaneId::Tom:        drawTom (g, r.reduced (r.getWidth() * 0.04f), c); break;
        case LaneId::Percussion: drawPercussion (g, r.reduced (r.getWidth() * 0.04f), c); break;
        case LaneId::Percussion2:drawCowbell (g, r.reduced (r.getWidth() * 0.04f), c); break;
        default: break;
    }
}

void Icons::drawSparkle (juce::Graphics& g, juce::Rectangle<float> area, juce::Colour c)
{
    auto r = area.withSizeKeepingCentre (juce::jmin (area.getWidth(), area.getHeight()),
                                         juce::jmin (area.getWidth(), area.getHeight()));

    // A four-point star drawn as a pinched diamond - the concave waist is what
    // makes it read as a sparkle and not as a rhombus.
    auto star = [&] (juce::Point<float> centre, float radius)
    {
        const float waist = radius * 0.26f;

        juce::Path p;
        p.startNewSubPath (centre.x, centre.y - radius);
        p.quadraticTo (centre.x, centre.y, centre.x + waist, centre.y - waist);
        p.quadraticTo (centre.x, centre.y, centre.x + radius, centre.y);
        p.quadraticTo (centre.x, centre.y, centre.x + waist, centre.y + waist);
        p.quadraticTo (centre.x, centre.y, centre.x, centre.y + radius);
        p.quadraticTo (centre.x, centre.y, centre.x - waist, centre.y + waist);
        p.quadraticTo (centre.x, centre.y, centre.x - radius, centre.y);
        p.quadraticTo (centre.x, centre.y, centre.x - waist, centre.y - waist);
        p.closeSubPath();

        g.fillPath (p);
    };

    g.setColour (c);
    star (r.getCentre().translated (-r.getWidth() * 0.10f, r.getHeight() * 0.06f),
          r.getWidth() * 0.42f);

    g.setColour (c.withAlpha (0.75f));
    star (r.getCentre().translated (r.getWidth() * 0.30f, -r.getHeight() * 0.28f),
          r.getWidth() * 0.19f);
}

void Icons::drawLock (juce::Graphics& g, juce::Rectangle<float> area, juce::Colour c, bool locked)
{
    auto r = area.withSizeKeepingCentre (juce::jmin (area.getWidth(), area.getHeight()),
                                         juce::jmin (area.getWidth(), area.getHeight()));

    const float line = juce::jmax (1.0f, r.getWidth() * 0.105f);

    // Body wide and low, shackle narrower and clearly above it. Keeping the
    // shackle inside the body's width is what makes the silhouette read as a
    // padlock instead of a bag with a handle.
    auto body = juce::Rectangle<float> (r.getCentreX() - r.getWidth() * 0.33f,
                                        r.getBottom() - r.getHeight() * 0.42f,
                                        r.getWidth() * 0.66f,
                                        r.getHeight() * 0.40f);

    g.setColour (c);
    g.fillRoundedRectangle (body, r.getWidth() * 0.12f);

    const float shackleR = r.getWidth() * 0.21f;
    const auto halfPi = juce::MathConstants<float>::halfPi;

    // Locked, the legs run into the body. Unlocked, the whole shackle lifts and
    // leaves a gap you can see - the same object in two states, rather than two
    // different drawings, so the difference is the message.
    const float lift = locked ? 0.0f : r.getHeight() * 0.14f;
    const float legBottom = body.getY() - lift;
    const float arcY = body.getY() - r.getHeight() * 0.16f - lift;

    juce::Path shackle;
    shackle.startNewSubPath (body.getCentreX() - shackleR, legBottom);
    shackle.lineTo (body.getCentreX() - shackleR, arcY);
    shackle.addCentredArc (body.getCentreX(), arcY, shackleR, shackleR, 0.0f, -halfPi, halfPi, false);
    shackle.lineTo (body.getCentreX() + shackleR, legBottom);

    g.strokePath (shackle, juce::PathStrokeType (line, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
}

void Icons::drawReload (juce::Graphics& g, juce::Rectangle<float> area, juce::Colour c)
{
    auto r = area.withSizeKeepingCentre (juce::jmin (area.getWidth(), area.getHeight()),
                                         juce::jmin (area.getWidth(), area.getHeight()));

    const float line = juce::jmax (1.0f, r.getWidth() * 0.135f);
    const float radius = r.getWidth() * 0.31f;
    const auto centre = r.getCentre();

    // Nearly a full turn, with the gap at the top where the arrowhead sits.
    // Angles here are measured clockwise from twelve o'clock.
    const float pi = juce::MathConstants<float>::pi;
    const float start = pi * 0.30f;
    const float end   = start + pi * 1.80f;

    juce::Path arc;
    arc.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, start, end, true);

    g.setColour (c);
    g.strokePath (arc, juce::PathStrokeType (line, juce::PathStrokeType::curved,
                                             juce::PathStrokeType::butt));

    // Built from the tangent and normal at the arc's end rather than by rotating
    // a shape: an AffineTransform here is easy to get ninety degrees out, and
    // this version cannot be.
    const juce::Point<float> tip (centre.x + radius * std::sin (end),
                                  centre.y - radius * std::cos (end));

    const juce::Point<float> tangent (std::cos (end), std::sin (end));
    const juce::Point<float> normal (-tangent.y, tangent.x);

    const float len = r.getWidth() * 0.30f;
    const float half = r.getWidth() * 0.19f;

    // The base sits slightly behind the arc end so the head grows out of the
    // stroke instead of floating off it.
    const juce::Point<float> base (tip.x - tangent.x * len * 0.28f,
                                   tip.y - tangent.y * len * 0.28f);

    juce::Path arrow;
    arrow.addTriangle (base.x + tangent.x * len,  base.y + tangent.y * len,
                       base.x + normal.x * half,  base.y + normal.y * half,
                       base.x - normal.x * half,  base.y - normal.y * half);
    g.fillPath (arrow);
}

void Icons::drawDice (juce::Graphics& g, juce::Rectangle<float> area, juce::Colour c)
{
    auto r = area.withSizeKeepingCentre (juce::jmin (area.getWidth(), area.getHeight()),
                                         juce::jmin (area.getWidth(), area.getHeight()));

    const float line = juce::jmax (1.0f, r.getWidth() * 0.09f);

    g.setColour (c);
    g.drawRoundedRectangle (r.reduced (line * 0.5f), r.getWidth() * 0.22f, line);

    // Five pips - the face that reads best at 14 pixels.
    const float d = r.getWidth() * 0.155f;
    const float ox = r.getWidth() * 0.235f;
    const float oy = r.getHeight() * 0.235f;

    for (auto p : { juce::Point<float> (-ox, -oy), juce::Point<float> (ox, -oy),
                    juce::Point<float> (0.0f, 0.0f),
                    juce::Point<float> (-ox,  oy), juce::Point<float> (ox,  oy) })
        g.fillEllipse (r.getCentreX() + p.x - d * 0.5f, r.getCentreY() + p.y - d * 0.5f, d, d);
}

} // namespace drumid::ui
