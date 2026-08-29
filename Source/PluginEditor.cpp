#include "PluginEditor.h"
#include "Engine/MidiExport.h"
#include "UI/Icons.h"

using namespace drumid;
using namespace drumid::ui;

// ============================================================================
//  MidiDragTile
// ============================================================================

MidiDragTile::MidiDragTile (DrumidAudioProcessor& p, int laneFilter, juce::String labelText)
    : proc (p), lane (laneFilter), text (std::move (labelText))
{
    setMouseCursor (juce::MouseCursor::DraggingHandCursor);
}

void MidiDragTile::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (0.5f);

    g.setColour (hover ? Colours::panelLight.brighter (0.2f) : Colours::panelLight);
    g.fillRoundedRectangle (b, 4.0f);
    g.setColour (hover ? Colours::accent : Colours::outline);
    g.drawRoundedRectangle (b, 4.0f, 1.0f);

    g.setColour (hover ? Colours::accent : Colours::text);
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText (text, getLocalBounds(), juce::Justification::centred, false);
}

void MidiDragTile::mouseEnter (const juce::MouseEvent&) { hover = true;  repaint(); }
void MidiDragTile::mouseExit  (const juce::MouseEvent&) { hover = false; repaint(); }

void MidiDragTile::mouseDrag (const juce::MouseEvent&)
{
    const auto& kit = proc.kit();

    juce::String name = "DRUMid_";
    name << genreName (proc.settings().genre) << "_" << kit.bars() << "bar_"
         << (lane < 0 ? juce::String ("kit") : juce::String (laneName ((LaneId) lane)))
         << "_" << proc.settings().seed;

    // The kit drag has to know which workflow it is serving. Under the
    // single-note map a flat export would stack all seven lanes on C3, so it
    // writes one track per lane instead - dropped into Session View, Ableton
    // makes a track per instrument.
    auto file = lane < 0 ? MidiExport::writeKitTempFile (kit, name)
                         : MidiExport::writeTempFile (kit, name, lane);

    if (file == juce::File())
        return;

    juce::DragAndDropContainer::performExternalDragDropOfFiles (
        juce::StringArray (file.getFullPathName()), false, this, nullptr);
}

// ============================================================================
//  BrandButton
// ============================================================================

void BrandButton::paintButton (juce::Graphics& g, bool highlighted, bool down)
{
    auto b = getLocalBounds().toFloat();

    g.setColour (Colours::accent.withMultipliedBrightness (down ? 0.85f : (highlighted ? 1.15f : 1.0f)));
    g.setFont (juce::FontOptions (21.0f, juce::Font::bold));

    const float markWidth = juce::GlyphArrangement::getStringWidth (
                                juce::Font (juce::FontOptions (21.0f, juce::Font::bold)), "DRUMid");

    g.drawText ("DRUMid", b.removeFromLeft (markWidth + 2.0f), juce::Justification::centredLeft, false);

    b.removeFromLeft (7.0f);

    auto pill = b.removeFromLeft (44.0f).withSizeKeepingCentre (44.0f, 15.0f);

    g.setColour (highlighted ? Colours::panelLight.brighter (0.2f) : Colours::panelLight);
    g.fillRoundedRectangle (pill, 3.0f);

    g.setColour (Colours::textDim);
    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    g.drawText (DRUMID_VERSION, pill, juce::Justification::centred, false);
}

// ============================================================================
//  IconButton
// ============================================================================

void IconButton::paintButton (juce::Graphics& g, bool highlighted, bool down)
{
    getLookAndFeel().drawButtonBackground (g, *this,
                                           findColour (juce::TextButton::buttonColourId),
                                           highlighted, down);

    auto b = getLocalBounds().toFloat();
    const float iconSize = juce::jmin (18.0f, b.getHeight() * 0.5f);
    const auto colour = findColour (getToggleState() ? juce::TextButton::textColourOnId
                                                     : juce::TextButton::textColourOffId);

    // Glyph and label are centred together rather than the label being centred
    // and the glyph hung off the edge.
    const float textWidth = juce::GlyphArrangement::getStringWidth (
                                juce::Font (juce::FontOptions (13.0f, juce::Font::bold)), getButtonText());
    const float gap = 8.0f;
    const float totalWidth = iconSize + gap + textWidth;
    const float startX = b.getCentreX() - totalWidth * 0.5f;

    if (painter != nullptr)
        painter (g, { startX, b.getCentreY() - iconSize * 0.5f, iconSize, iconSize }, colour);

    g.setColour (colour);
    g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    g.drawText (getButtonText(),
                juce::Rectangle<float> (startX + iconSize + gap, b.getY(), textWidth + 2.0f, b.getHeight()),
                juce::Justification::centredLeft, false);
}

// ============================================================================
//  Editor
// ============================================================================

DrumidAudioProcessorEditor::DrumidAudioProcessorEditor (DrumidAudioProcessor& p)
    : AudioProcessorEditor (&p), proc (p),
      generateButton ("GENERATE", [] (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c)
                                  { Icons::drawDice (g, r, c); }),
      surpriseButton ("SURPRISE", [] (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c)
                                  { Icons::drawSparkle (g, r, c); }),
      grid (p), kitDrag (p, -1, "DRAG KIT MIDI")
{
    setLookAndFeel (&lnf);

    buildControls();

    addAndMakeVisible (grid);
    addAndMakeVisible (kitDrag);

    aboutPanel.setVisible (false);
    addChildComponent (aboutPanel);

    grid.onEdit = [this]
    {
        proc.publishKit();
    };

    grid.onNoteEdited = [this]
    {
        proc.publishKit();
        noteMapBox.setSelectedId ((int) NoteMapPreset::Custom + 1, juce::dontSendNotification);
    };

    proc.kitChanged.addChangeListener (this);

    setResizable (true, true);
    setResizeLimits (820, 470, 1600, 980);
    setSize (1020, 552);

    refreshFromProcessor();
    startTimerHz (30);
}

DrumidAudioProcessorEditor::~DrumidAudioProcessorEditor()
{
    proc.kitChanged.removeChangeListener (this);
    setLookAndFeel (nullptr);
}

juce::Slider& DrumidAudioProcessorEditor::addKnob (juce::Slider& s, juce::Label& l,
                                                   const juce::String& name,
                                                   double min, double max, double interval)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 54, 15);
    s.setRange (min, max, interval);
    addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setColour (juce::Label::textColourId, Colours::textDim);
    l.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    addAndMakeVisible (l);

    return s;
}

void DrumidAudioProcessorEditor::buildControls()
{
    brandButton.onClick = [this] { showAbout (true); };
    brandButton.setTooltip ("About DRUMid");
    addAndMakeVisible (brandButton);

    hintLabel.setText ("the reload arrow redoes just that drum  -  the padlock keeps GENERATE away from it  -  "
                       "drag DYN for how much its velocity moves  -  click the name to disable  -  "
                       "drag the icon to export the lane  -  alt+drag a step for velocity  -  "
                       "double click for ratchet",
                       juce::dontSendNotification);
    hintLabel.setFont (juce::FontOptions (10.5f));
    hintLabel.setColour (juce::Label::textColourId, Colours::textDim);
    addAndMakeVisible (hintLabel);

    seedLabel.setFont (juce::FontOptions (11.0f));
    seedLabel.setColour (juce::Label::textColourId, Colours::textDim);
    seedLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (seedLabel);

    for (int i = 0; i < (int) Genre::NumGenres; ++i)
        genreBox.addItem (genreName ((Genre) i), i + 1);

    genreBox.onChange = [this]
    {
        proc.settings().genre = (Genre) (genreBox.getSelectedId() - 1);

        // Each style has its own natural feel, so the swing follows the genre
        // unless you move it yourself afterwards.
        proc.settings().swing = defaultSwingFor (proc.settings().genre);

        swing.setValue (proc.settings().swing, juce::dontSendNotification);
        proc.generateAll (true);
    };
    addAndMakeVisible (genreBox);

    for (int b : { 1, 2, 4 })
        barsBox.addItem (juce::String (b) + (b == 1 ? " bar" : " bars"), b);

    barsBox.onChange = [this]
    {
        proc.settings().bars = barsBox.getSelectedId();
        proc.generateAll (false);
    };
    addAndMakeVisible (barsBox);

    for (int i = 0; i < (int) NoteMapPreset::NumPresets; ++i)
        noteMapBox.addItem (NoteMap::presetName ((NoteMapPreset) i), i + 1);

    noteMapBox.onChange = [this]
    {
        proc.setNoteMapPreset ((NoteMapPreset) (noteMapBox.getSelectedId() - 1));
        grid.repaint();
    };
    addAndMakeVisible (noteMapBox);

    generateButton.onClick = [this] { proc.generateAll (true); };
    generateButton.setColour (juce::TextButton::buttonColourId, Colours::accent);
    generateButton.setColour (juce::TextButton::textColourOffId, Colours::background);
    addAndMakeVisible (generateButton);

    // GENERATE rerolls the patterns you asked for; SURPRISE rerolls what you
    // asked for as well - genre, energy, complexity, feel.
    surpriseButton.onClick = [this] { proc.randomizeAll(); };
    surpriseButton.setColour (juce::TextButton::buttonColourId, Colours::panelLight);
    surpriseButton.setColour (juce::TextButton::textColourOffId, Colours::text);
    addAndMakeVisible (surpriseButton);

    fillsButton.onClick = [this]
    {
        proc.settings().fills = fillsButton.getToggleState();
        proc.generateAll (false);
    };
    addAndMakeVisible (fillsButton);

    addKnob (energy,      energyLbl,      "ENERGY",   0.0, 1.0, 0.01);
    addKnob (complexity,  complexityLbl,  "COMPLEX",  0.0, 1.0, 0.01);
    addKnob (swing,       swingLbl,       "SWING",    0.5, 0.70, 0.005);
    addKnob (humanTiming, humanTimingLbl, "TIMING",   0.0, 1.0, 0.01);
    addKnob (humanVel,    humanVelLbl,    "DYNAMICS", 0.0, 1.0, 0.01);

    energy.onValueChange      = [this] { proc.settings().energy      = (float) energy.getValue();      proc.generateAll (false); };
    complexity.onValueChange  = [this] { proc.settings().complexity  = (float) complexity.getValue();  proc.generateAll (false); };
    swing.onValueChange       = [this] { proc.settings().swing       = (float) swing.getValue();       proc.generateAll (false); };
    humanTiming.onValueChange = [this] { proc.settings().humanTiming = (float) humanTiming.getValue(); proc.generateAll (false); };
    humanVel.onValueChange    = [this] { proc.settings().humanVel    = (float) humanVel.getValue();    proc.generateAll (false); };
}

void DrumidAudioProcessorEditor::refreshFromProcessor()
{
    const auto& s = proc.settings();

    genreBox.setSelectedId ((int) s.genre + 1, juce::dontSendNotification);
    barsBox.setSelectedId (s.bars, juce::dontSendNotification);
    noteMapBox.setSelectedId ((int) proc.noteMapPreset() + 1, juce::dontSendNotification);

    energy.setValue (s.energy, juce::dontSendNotification);
    complexity.setValue (s.complexity, juce::dontSendNotification);
    swing.setValue (s.swing, juce::dontSendNotification);
    humanTiming.setValue (s.humanTiming, juce::dontSendNotification);
    humanVel.setValue (s.humanVel, juce::dontSendNotification);
    fillsButton.setToggleState (s.fills, juce::dontSendNotification);

    seedLabel.setText ("seed " + juce::String (s.seed), juce::dontSendNotification);
    grid.repaint();
}

void DrumidAudioProcessorEditor::changeListenerCallback (juce::ChangeBroadcaster*)
{
    refreshFromProcessor();
}

void DrumidAudioProcessorEditor::showAbout (bool shouldBeVisible)
{
    aboutPanel.setBounds (getLocalBounds());
    aboutPanel.setVisible (shouldBeVisible);

    if (shouldBeVisible)
        aboutPanel.toFront (false);
}

void DrumidAudioProcessorEditor::timerCallback()
{
    grid.setPlayStep (proc.currentStep());
}

void DrumidAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (Colours::background);

    g.setColour (Colours::panel);
    g.fillRect (0, 0, getWidth(), 44);
    g.fillRect (0, 44, getWidth(), 78);

    g.setColour (Colours::outline);
    g.drawHorizontalLine (44, 0.0f, (float) getWidth());
    g.drawHorizontalLine (122, 0.0f, (float) getWidth());
}

void DrumidAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();

    // ---- top bar
    aboutPanel.setBounds (getLocalBounds());

    auto top = area.removeFromTop (44).reduced (10, 7);
    brandButton.setBounds (top.removeFromLeft (152));
    genreBox.setBounds (top.removeFromLeft (150).reduced (0, 1));
    top.removeFromLeft (8);
    barsBox.setBounds (top.removeFromLeft (80).reduced (0, 1));
    top.removeFromLeft (8);
    noteMapBox.setBounds (top.removeFromLeft (140).reduced (0, 1));
    seedLabel.setBounds (top.removeFromRight (90));

    // ---- control strip
    auto controls = area.removeFromTop (78).reduced (10, 6);

    auto buttons = controls.removeFromLeft (134);
    generateButton.setBounds (buttons.removeFromTop (38));
    buttons.removeFromTop (5);
    surpriseButton.setBounds (buttons.removeFromTop (23));
    controls.removeFromLeft (14);

    auto knobArea = controls.removeFromLeft (5 * 76);

    auto placeKnob = [&] (juce::Slider& s, juce::Label& l)
    {
        auto col = knobArea.removeFromLeft (76);
        l.setBounds (col.removeFromTop (12));
        s.setBounds (col);
    };

    placeKnob (energy, energyLbl);
    placeKnob (complexity, complexityLbl);
    placeKnob (swing, swingLbl);
    placeKnob (humanTiming, humanTimingLbl);
    placeKnob (humanVel, humanVelLbl);

    controls.removeFromLeft (10);
    fillsButton.setBounds (controls.removeFromLeft (70).withSizeKeepingCentre (70, 24));

    kitDrag.setBounds (controls.removeFromRight (128).withSizeKeepingCentre (128, 30));

    // ---- footer hint
    hintLabel.setBounds (area.removeFromBottom (20).reduced (12, 2));

    grid.setBounds (area.reduced (10, 6));
}
