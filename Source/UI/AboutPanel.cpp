#include "AboutPanel.h"
#include "Icons.h"
#include "../Engine/Types.h"
#include "../Engine/PatternLibrary.h"

namespace drumid::ui
{

AboutPanel::AboutPanel()
{
    closeButton.onClick = [this] { setVisible (false); };
    closeButton.setColour (juce::TextButton::buttonColourId, Colours::panelLight);
    addAndMakeVisible (closeButton);

    setAlwaysOnTop (true);
}

juce::Rectangle<float> AboutPanel::cardArea() const
{
    const auto width  = (float) juce::jmin (420, juce::jmax (340, getWidth()  - 80));
    const auto height = (float) juce::jmin (340, juce::jmax (280, getHeight() - 60));

    return juce::Rectangle<float> (width, height).withCentre (getLocalBounds().toFloat().getCentre());
}

void AboutPanel::resized()
{
    auto card = cardArea().toNearestInt();
    closeButton.setBounds (card.removeFromBottom (46).reduced (22, 9));
}

void AboutPanel::mouseDown (const juce::MouseEvent& e)
{
    // Clicking the dimmed area outside the card closes it, the way any modal
    // ought to behave.
    if (! cardArea().contains (e.position))
        setVisible (false);
}

void AboutPanel::paint (juce::Graphics& g)
{
    g.fillAll (Colours::background.withAlpha (0.86f));

    auto card = cardArea();

    g.setColour (Colours::panel);
    g.fillRoundedRectangle (card, 8.0f);
    g.setColour (Colours::outline);
    g.drawRoundedRectangle (card.reduced (0.5f), 8.0f, 1.0f);

    auto content = card.reduced (24.0f, 22.0f).withTrimmedBottom (42.0f);

    g.setColour (Colours::textDim);
    g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
    g.drawText (juce::String (DRUMID_COMPANY).toUpperCase(), content.removeFromTop (14.0f),
                juce::Justification::centredLeft, false);

    content.removeFromTop (6.0f);

    // Wordmark, with the lane glyphs beside it as a signature.
    auto titleRow = content.removeFromTop (26.0f);
    auto glyphs = titleRow.removeFromRight (86.0f);

    g.setColour (Colours::accent);
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText ("DRUMid", titleRow, juce::Justification::centredLeft, false);

    for (auto lane : { LaneId::Kick, LaneId::ClosedHat, LaneId::Percussion })
    {
        auto slot = glyphs.removeFromLeft (28.0f);
        Icons::drawLane (g, lane, slot.withSizeKeepingCentre (18.0f, 18.0f),
                         Colours::textDim);
    }

    g.setColour (Colours::text);
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText ("MIDI DRUM PATTERN GENERATOR", content.removeFromTop (18.0f),
                juce::Justification::centredLeft, false);

    content.removeFromTop (12.0f);
    g.setColour (Colours::outline);
    g.fillRect (content.removeFromTop (1.0f));
    content.removeFromTop (14.0f);

    auto row = [&g, &content] (const juce::String& label, const juce::String& value)
    {
        auto line = content.removeFromTop (20.0f);

        g.setColour (Colours::textDim);
        g.setFont (juce::FontOptions (11.5f));
        g.drawText (label, line.removeFromLeft (86.0f), juce::Justification::centredLeft, false);

        g.setColour (Colours::text);
        g.setFont (juce::FontOptions (12.5f));
        g.drawText (value, line, juce::Justification::centredLeft, false);
    };

    row ("Version", DRUMID_VERSION);
    row ("Format",  "VST3 - universal arm64 + x86_64");
    row ("Patterns", juce::String (PatternLibrary::size()) + " curated seeds");

    content.removeFromTop (14.0f);

    g.setColour (Colours::text);
    g.setFont (juce::FontOptions (12.0f));
    g.drawFittedText ("Drum patterns for melodic and organic house, afro house, indie dance "
                      "and techno. Pick your lanes, generate, drag the MIDI to your racks.",
                      content.removeFromTop (36.0f).toNearestInt(), juce::Justification::topLeft, 3);

    content.removeFromTop (8.0f);

    g.setColour (Colours::textDim);
    g.setFont (juce::FontOptions (11.0f));
    g.drawFittedText (juce::String ((int) Genre::NumGenres) + " genres - "
                        + juce::String (kNumLanes) + " lanes - up to 4 bars\n"
                        "Built with JUCE",
                      content.toNearestInt(), juce::Justification::topLeft, 3);
}

} // namespace drumid::ui
