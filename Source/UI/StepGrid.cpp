#include "StepGrid.h"
#include "../Engine/NoteMap.h"
#include "Icons.h"
#include "../Engine/MidiExport.h"

namespace drummy::ui
{

static constexpr int kIconX   = 8;
static constexpr int kIconW   = 22;
static constexpr int kNameX   = 38;
static constexpr int kNameW   = 78;
static constexpr int kNoteX   = 120;
static constexpr int kNoteW   = 34;
static constexpr int kLockX   = 160;
static constexpr int kReroll  = 182;
static constexpr int kBtnW    = 18;

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

        if (lane > 0)
        {
            g.setColour (Colours::outline.withAlpha (0.5f));
            g.fillRect (row.getX(), row.getY(), row.getWidth(), 1);
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

    const bool dim = ! ls.enabled;

    Icons::drawLane (g, id,
                     juce::Rectangle<float> ((float) kIconX,
                                             (float) row.getY() + (float) row.getHeight() * 0.5f - (float) kIconW * 0.5f,
                                             (float) kIconW, (float) kIconW),
                     dim ? Colours::textDim : Colours::accent);

    g.setColour (dim ? Colours::textDim : Colours::text);
    g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    g.drawText (laneName (id), kNameX, row.getY(), kNameW, row.getHeight(),
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
    // you can see and not just hear. The shift is capped well short of the cell
    // edge - past that the blocks merge with their neighbours and the grid stops
    // being readable, which costs more than the extra precision is worth.
    const float shift = juce::jlimit (-b.getWidth() * 0.28f, b.getWidth() * 0.28f,
                                      st.micro * b.getWidth());

    auto hit = b.reduced (1.0f).translated (shift, 0.0f);

    const float vel = juce::jlimit (0.0f, 1.0f, st.velocity);
    const bool dim = ! ls.enabled;

    auto colour = (vel < 0.5f ? Colours::accentSoft : Colours::accent)
                      .withMultipliedAlpha (dim ? 0.35f : 1.0f);

    // Height encodes velocity so the accent pattern reads at a glance, over a
    // narrow enough range that ghosts stay obviously present rather than
    // looking like empty steps.
    const float h = hit.getHeight() * juce::jmap (vel, 0.0f, 1.0f, 0.5f, 1.0f);
    auto bar = hit.withHeight (h).withY (hit.getBottom() - h);

    // A dim full-height footprint keeps the step's own slot visible even when
    // the velocity block is short and shifted.
    g.setColour (colour.withAlpha (0.16f));
    g.fillRoundedRectangle (hit, 2.0f);

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
        g.setColour (Colours::text.withAlpha (0.5f));
        g.drawRoundedRectangle (bar.reduced (0.5f), 2.0f, 1.0f);
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
        else if (pos.x >= kReroll && pos.x < kReroll + kBtnW) h.kind = Hit::Reroll;
        else if (pos.x >= kNoteX  && pos.x < kNoteX  + kNoteW) h.kind = Hit::Note;
        else if (pos.x >= kIconX  && pos.x < kIconX  + kIconW) h.kind = Hit::Icon;
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

        case Hit::Name:
            kit.lanes[(size_t) h.lane].enabled = ! kit.lanes[(size_t) h.lane].enabled;
            break;

        case Hit::Reroll:
            proc.generateLane ((LaneId) h.lane);
            break;

        case Hit::Note:
            dragStartNote = kit.lanes[(size_t) h.lane].midiNote;
            return;

        case Hit::Icon:
            return;   // a press on the icon starts a MIDI drag, it is not a click

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

    // Dragging a lane's icon exports that lane on its own, which is how one
    // instrument gets onto one track.
    if (dragOrigin.kind == Hit::Icon)
    {
        if (e.getDistanceFromDragStart() < 5)
            return;

        juce::String name;
        name << "Drummy_" << genreName (proc.settings().genre)
             << "_" << kit.bars() << "bar_"
             << laneName ((LaneId) dragOrigin.lane)
             << "_" << proc.settings().seed;

        auto file = MidiExport::writeTempFile (kit, name, dragOrigin.lane);

        if (file == juce::File())
            return;

        dragOrigin = Hit {};   // one drag per press

        juce::DragAndDropContainer::performExternalDragDropOfFiles (
            juce::StringArray (file.getFullPathName()), false, this, nullptr);

        return;
    }

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
