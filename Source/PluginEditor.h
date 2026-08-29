#pragma once

#include "PluginProcessor.h"
#include "UI/StepGrid.h"
#include "UI/DrumidLookAndFeel.h"
#include "UI/AboutPanel.h"

/** A drag source that hands the host a .mid file.

    This is the workflow that actually matters in Ableton: generate, listen,
    then drag the bars straight into a clip slot. Dragging the "KIT" tile
    exports every lane on its mapped note; dragging a lane name exports that
    lane alone.
*/
class MidiDragTile : public juce::Component
{
public:
    MidiDragTile (DrumidAudioProcessor& p, int laneFilter, juce::String labelText);

    void paint (juce::Graphics&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit (const juce::MouseEvent&) override;

private:
    DrumidAudioProcessor& proc;
    int lane;
    juce::String text;
    bool hover = false;
};

/** The wordmark, with the version beside it. Clicking it opens the about card,
    which is where the version lives so it never has to compete for header room. */
class BrandButton : public juce::Button
{
public:
    BrandButton() : juce::Button ("about") {}
    void paintButton (juce::Graphics&, bool highlighted, bool down) override;
};

/** A TextButton that carries a glyph next to its label. */
class IconButton : public juce::TextButton
{
public:
    using Painter = std::function<void (juce::Graphics&, juce::Rectangle<float>, juce::Colour)>;

    IconButton (const juce::String& text, Painter p)
        : juce::TextButton (text), painter (std::move (p)) {}

    void paintButton (juce::Graphics&, bool highlighted, bool down) override;

private:
    Painter painter;
};

class DrumidAudioProcessorEditor : public juce::AudioProcessorEditor,
                                   private juce::Timer,
                                   private juce::ChangeListener
{
public:
    explicit DrumidAudioProcessorEditor (DrumidAudioProcessor&);
    ~DrumidAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    /** Public so the offscreen preview renderer can capture the about card. */
    void showAbout (bool shouldBeVisible);

private:
    void timerCallback() override;
    void changeListenerCallback (juce::ChangeBroadcaster*) override;

    void buildControls();
    void refreshFromProcessor();
    juce::Slider& addKnob (juce::Slider& s, juce::Label& l, const juce::String& name,
                           double min, double max, double interval);

    DrumidAudioProcessor& proc;
    drumid::ui::DrumidLookAndFeel lnf;

    BrandButton       brandButton;
    juce::Label       seedLabel, hintLabel;
    juce::ComboBox    genreBox, barsBox, noteMapBox;
    IconButton        generateButton;
    IconButton        surpriseButton;
    juce::TextButton  playButton { "PLAY" };
    juce::ToggleButton fillsButton { "Fills" };

    juce::Slider energy, complexity, swing, humanTiming, humanVel;
    juce::Label  energyLbl, complexityLbl, swingLbl, humanTimingLbl, humanVelLbl;

    drumid::ui::StepGrid grid;
    MidiDragTile kitDrag;
    drumid::ui::AboutPanel aboutPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumidAudioProcessorEditor)
};
