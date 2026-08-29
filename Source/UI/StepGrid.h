#pragma once

#include "../PluginProcessor.h"
#include "DrummyLookAndFeel.h"

namespace drummy::ui
{

/** Lane header + step grid.

    Header and cells live in one component so the rows can never drift out of
    alignment. Interaction:
      click / drag      paint steps on and off
      alt + drag up     raise velocity, alt + drag down lower it
      double click      cycle the ratchet (1 -> 2 -> 3 -> 1)
      drag the note     retune the lane, which switches the map to Custom
*/
class StepGrid : public juce::Component
{
public:
    explicit StepGrid (DrummyAudioProcessor& p);

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

    void setPlayStep (int step);

    std::function<void()> onEdit;          // any change the processor must publish
    std::function<void()> onNoteEdited;    // lane note changed -> switch to Custom

    static constexpr int headerWidth = 190;

private:
    struct Hit
    {
        enum Kind { None, Cell, Lock, Mute, Reroll, Note, Name } kind = None;
        int lane = -1;
        int step = -1;
    };

    Hit hitTest (juce::Point<int> pos) const;
    juce::Rectangle<int> cellBounds (int lane, int step) const;
    float rowHeight() const;
    float cellWidth() const;
    void drawHeader (juce::Graphics&, int lane, juce::Rectangle<int> row);
    void drawCell (juce::Graphics&, int lane, int step);

    DrummyAudioProcessor& proc;

    int playStep = -1;
    Hit dragOrigin;
    bool paintingOn = true;
    int dragStartNote = 36;
    float dragStartVel = 0.8f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StepGrid)
};

} // namespace drummy::ui
