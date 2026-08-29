#include "StepGrid.h"
#include "../Engine/NoteMap.h"

namespace drummy::ui
{

static constexpr int kLockX   = 136;
static constexpr int kMuteX   = 154;
static constexpr int kReroll  = 172;
static constexpr int kBtnW    = 16;
static constexpr int kNoteX   = 98;
static constexpr int kNoteW   = 34;

StepGrid::StepGrid (DrummyAudioProcessor& p) : proc (p)
{
    setWantsKeyboardFocus (false);
}

float StepGrid::rowHeight() const
{
    return (float) getHeight() / (float) kNumLanes;
}

float StepGrid::cellWidth() const
{
    const int steps = juce::jmax (1, proc.kit().numSteps);
    return (float) (getWidth() - headerWidth) / (float) steps;
}

juce::Rectangle<int> StepGrid::cellBounds (int lane, int step) const
{
    const float rh = rowHeight();
    const float cw = cellWidth();

    return juce::Rectangle<float> (headerWidth + step * cw, lane * rh, cw, rh)
               .reduced (1.5f, 3.0f)
               .toNearestInt();
}

// ============================================================================

void StepGrid::paint (juce::Graphics& g)
{
    auto& kit = proc.kit();
    const int steps = juce::jmax (1, kit.numSteps);
    const float rh = rowHeight();
    const float cw = cellWidth();

    g.fillAll (Colours::background);

    // playhead column
    if (playStep >= 0 && playStep < steps)
    {
        g.setColour (Colours::playhead.withAlpha (0.12f));
        g.fillRect (juce::Rectangle<float> (headerWidth + playStep * cw, 0.0f, cw, (float) getHeight()));
    }

    for (int lane = 0; lane < kNumLanes; ++lane)
    {
        auto row = juce::Rectangle<float> (0.0f, lane * rh, (float) getWidth(), rh).toNearestInt();

        if (lane % 2 == 1)
        {
            g.setColour (Colours::panel.withAlpha (0.45f));
            g.fillRect (row);
        }

        drawHeader (g, lane, row);

        for (int s = 0; s < steps; ++s)
            drawCell (g, lane, s);
    }

    // bar and beat dividers, drawn last so they sit on top of the cells
    for (int s = 0; s <= steps; ++s)
    {
        const float x = headerWidth + s * cw;
        const bool bar  = (s % kStepsPerBar) == 0;
        const bool beat = (s % 4) == 0;

        if (! beat)
            continue;

        g.setColour (bar ? Colours::outline.brighter (0.35f) : Colours::outline.withAlpha (0.55f));
        g.fillRect (x - 0.5f, 0.0f, bar ? 1.5f : 1.0f, (float) getHeight());
    }

    g.setColour (Colours::outline);
    g.drawVerticalLine (headerWidth - 1, 0.0f, (float) getHeight());
}

void StepGrid::drawHeader (juce::Graphics& g, int lane, juce::Rectangle<int> row)
{
    const auto& ls = proc.kit().lanes[(size_t) lane];
    const auto id = (LaneId) lane;

    const bool dim = ! ls.enabled || ls.muted;

    g.setColour (dim ? Colours::textDim : Colours::text);
    g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    g.drawText (laneName (id), row.getX() + 8, row.getY(), 86, row.getHeight(),
                juce::Justification::centredLeft, true);

    // note pad
    auto noteBox = juce::Rectangle<int> (kNoteX, row.getY() + row.getHeight() / 2 - 9, kNoteW, 18);
    g.setColour (Colours::panelLight);
    g.fillRoundedRectangle (noteBox.toFloat(), 3.0f);
    g.setColour (Colours::textDim);
    g.setFont (juce::FontOptions (11.0f));
    g.drawText (NoteMap::noteName (ls.midiNote), noteBox, juce::Justification::centred, false);

    auto badge = [&] (int x, const juce::String& text, bool on, juce::Colour onColour)
    {
        auto b = juce::Rectangle<int> (x, row.getY() + row.getHeight() / 2 - 8, kBtnW, 16);
        g.setColour (on ? onColour : Colours::panelLight);
        g.fillRoundedRectangle (b.toFloat(), 3.0f);
        g.setColour (on ? Colours::background : Colours::textDim);
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.drawText (text, b, juce::Justification::centred, false);
    };

    badge (kLockX,  "L", ls.locked, Colours::locked);
    badge (kMuteX,  "M", ls.muted,  Colours::danger);
    badge (kReroll, "R", false,     Colours::accent);
}

void StepGrid::drawCell (juce::Graphics& g, int lane, int step)
{
    const auto& kit = proc.kit();
    const auto& st  = kit.patterns[(size_t) lane].steps[(size_t) step];
    const auto& ls  = kit.lanes[(size_t) lane];

    auto b = cellBounds (lane, step).toFloat();

    g.setColour (Colours::panel);
    g.fillRoundedRectangle (b, 2.5f);

    if (! st.on)
        return;

    // Micro-timing shifts the block visibly, so swing and humanize are something
    // you can see and not just hear.
    const float shift = st.micro * b.getWidth();
    auto hit = b.reduced (1.0f).translated (juce::jlimit (-b.getWidth() * 0.45f,
                                                          b.getWidth() * 0.45f, shift), 0.0f);

    const float vel = juce::jlimit (0.0f, 1.0f, st.velocity);
    const bool dim = ! ls.enabled || ls.muted;

    auto colour = (vel < 0.5f ? Colours::accentSoft : Colours::accent)
                      .withMultipliedAlpha (dim ? 0.35f : 1.0f);

    // Height encodes velocity - the accent pattern is readable at a glance.
    const float h = hit.getHeight() * juce::jmap (vel, 0.0f, 1.0f, 0.35f, 1.0f);
    auto bar = hit.withHeight (h).withY (hit.getBottom() - h);

    g.setColour (colour);
    g.fillRoundedRectangle (bar, 2.0f);

    if (st.ratchet > 1)
    {
        g.setColour (Colours::background.withAlpha (0.7f));

        for (int r = 1; r < st.ratchet; ++r)
        {
            const float x = bar.getX() + bar.getWidth() * (float) r / (float) st.ratchet;
            g.fillRect (x - 0.5f, bar.getY(), 1.0f, bar.getHeight());
        }
    }

    if (st.probability < 0.999f)
    {
        g.setColour (Colours::text.withAlpha (0.55f));
        g.drawRoundedRectangle (hit.reduced (0.5f), 2.0f, 1.0f);
    }
}

// ============================================================================

StepGrid::Hit StepGrid::hitTest (juce::Point<int> pos) const
{
    Hit h;

    const float rh = rowHeight();

    if (rh <= 0.0f)
        return h;

    const int lane = juce::jlimit (0, kNumLanes - 1, (int) ((float) pos.y / rh));
    h.lane = lane;

    if (pos.x < headerWidth)
    {
        if (pos.x >= kLockX  && pos.x < kLockX  + kBtnW) h.kind = Hit::Lock;
        else if (pos.x >= kMuteX  && pos.x < kMuteX  + kBtnW) h.kind = Hit::Mute;
        else if (pos.x >= kReroll && pos.x < kReroll + kBtnW) h.kind = Hit::Reroll;
        else if (pos.x >= kNoteX  && pos.x < kNoteX  + kNoteW) h.kind = Hit::Note;
        else h.kind = Hit::Name;

        return h;
    }

    const float cw = cellWidth();

    if (cw <= 0.0f)
        return h;

    const int step = (int) ((float) (pos.x - headerWidth) / cw);

    if (step < 0 || step >= proc.kit().numSteps)
        return h;

    h.kind = Hit::Cell;
    h.step = step;
    return h;
}

void StepGrid::mouseDown (const juce::MouseEvent& e)
{
    auto h = hitTest (e.getPosition());
    dragOrigin = h;

    auto& kit = proc.kit();

    switch (h.kind)
    {
        case Hit::Lock:
            kit.lanes[(size_t) h.lane].locked = ! kit.lanes[(size_t) h.lane].locked;
            break;

        case Hit::Mute:
            kit.lanes[(size_t) h.lane].muted = ! kit.lanes[(size_t) h.lane].muted;
            break;

        case Hit::Name:
            kit.lanes[(size_t) h.lane].enabled = ! kit.lanes[(size_t) h.lane].enabled;
            break;

        case Hit::Reroll:
            proc.generateLane ((LaneId) h.lane);
            break;

        case Hit::Note:
            dragStartNote = kit.lanes[(size_t) h.lane].midiNote;
            return;

        case Hit::Cell:
        {
            auto& st = kit.patterns[(size_t) h.lane].steps[(size_t) h.step];

            if (e.mods.isAltDown())
            {
                dragStartVel = st.velocity;
                return;
            }

            paintingOn = ! st.on;

            if (paintingOn)
                st = Step { true, 0.8f, 0.0f, 1.0f, 1 };
            else
                st.clear();

            break;
        }

        default:
            return;
    }

    repaint();

    if (onEdit != nullptr)
        onEdit();
}

void StepGrid::mouseDrag (const juce::MouseEvent& e)
{
    auto& kit = proc.kit();

    if (dragOrigin.kind == Hit::Note)
    {
        const int delta = -e.getDistanceFromDragStartY() / 6;
        kit.lanes[(size_t) dragOrigin.lane].midiNote = juce::jlimit (0, 127, dragStartNote + delta);

        repaint();

        if (onNoteEdited != nullptr)
            onNoteEdited();

        return;
    }

    if (dragOrigin.kind != Hit::Cell)
        return;

    if (e.mods.isAltDown())
    {
        auto& st = kit.patterns[(size_t) dragOrigin.lane].steps[(size_t) dragOrigin.step];
        st.velocity = juce::jlimit (0.05f, 1.0f, dragStartVel - (float) e.getDistanceFromDragStartY() / 140.0f);
        st.on = true;
    }
    else
    {
        auto h = hitTest (e.getPosition());

        if (h.kind != Hit::Cell)
            return;

        auto& st = kit.patterns[(size_t) h.lane].steps[(size_t) h.step];

        if (paintingOn)
        {
            if (! st.on)
                st = Step { true, 0.8f, 0.0f, 1.0f, 1 };
        }
        else
        {
            st.clear();
        }
    }

    repaint();

    if (onEdit != nullptr)
        onEdit();
}

void StepGrid::mouseUp (const juce::MouseEvent&)
{
    dragOrigin = Hit {};
}

void StepGrid::mouseDoubleClick (const juce::MouseEvent& e)
{
    auto h = hitTest (e.getPosition());

    if (h.kind != Hit::Cell)
        return;

    auto& st = proc.kit().patterns[(size_t) h.lane].steps[(size_t) h.step];

    if (! st.on)
        return;

    st.ratchet = st.ratchet >= 3 ? 1 : st.ratchet + 1;

    repaint();

    if (onEdit != nullptr)
        onEdit();
}

void StepGrid::setPlayStep (int step)
{
    if (step == playStep)
        return;

    playStep = step;
    repaint();
}

} // namespace drummy::ui
