#include "DrummyLookAndFeel.h"

namespace drummy::ui
{

DrummyLookAndFeel::DrummyLookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, Colours::background);
    setColour (juce::Label::textColourId,                 Colours::text);
    setColour (juce::Slider::textBoxTextColourId,         Colours::text);
    setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId,   Colours::panel);
    setColour (juce::TextButton::textColourOffId,         Colours::text);
    setColour (juce::TextButton::textColourOnId,          Colours::background);
    setColour (juce::ComboBox::textColourId,              Colours::text);
    setColour (juce::ComboBox::backgroundColourId,        Colours::panel);
    setColour (juce::ComboBox::arrowColourId,             Colours::textDim);
    setColour (juce::ComboBox::outlineColourId,           Colours::outline);
    setColour (juce::PopupMenu::backgroundColourId,       Colours::panel);
    setColour (juce::PopupMenu::textColourId,             Colours::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, Colours::accent);
    setColour (juce::PopupMenu::highlightedTextColourId,  Colours::background);
    setColour (juce::ToggleButton::textColourId,          Colours::text);
    setColour (juce::ToggleButton::tickColourId,          Colours::accent);
}

void DrummyLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                          float sliderPos, float startAngle, float endAngle,
                                          juce::Slider&)
{
    auto bounds = juce::Rectangle<int> (x, y, w, h).toFloat().reduced (3.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle  = startAngle + sliderPos * (endAngle - startAngle);
    const auto thickness = juce::jmax (2.5f, radius * 0.22f);

    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius - thickness, radius - thickness,
                         0.0f, startAngle, endAngle, true);
    g.setColour (Colours::panelLight);
    g.strokePath (track, juce::PathStrokeType (thickness, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    juce::Path value;
    value.addCentredArc (centre.x, centre.y, radius - thickness, radius - thickness,
                         0.0f, startAngle, angle, true);
    g.setColour (Colours::accent);
    g.strokePath (value, juce::PathStrokeType (thickness, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));

    juce::Path pointer;
    pointer.startNewSubPath (centre.x, centre.y - (radius - thickness * 1.8f));
    pointer.lineTo (centre.x, centre.y - (radius - thickness * 0.2f));
    pointer.applyTransform (juce::AffineTransform::rotation (angle, centre.x, centre.y));
    g.setColour (Colours::text);
    g.strokePath (pointer, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
}

void DrummyLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b,
                                              const juce::Colour& base,
                                              bool highlighted, bool down)
{
    auto bounds = b.getLocalBounds().toFloat().reduced (0.5f);
    const bool on = b.getToggleState();

    auto fill = on ? Colours::accent : base;

    if (down)        fill = fill.darker (0.25f);
    else if (highlighted) fill = fill.brighter (0.12f);

    g.setColour (fill);
    g.fillRoundedRectangle (bounds, 4.0f);

    g.setColour (on ? Colours::accent.brighter (0.3f) : Colours::outline);
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}

void DrummyLookAndFeel::drawComboBox (juce::Graphics& g, int w, int h, bool,
                                      int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) w, (float) h).reduced (0.5f);

    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (bounds, 4.0f);
    g.setColour (Colours::outline);
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

    juce::Path arrow;
    const float cx = (float) w - 14.0f;
    const float cy = (float) h * 0.5f;
    arrow.startNewSubPath (cx - 4.0f, cy - 2.0f);
    arrow.lineTo (cx, cy + 3.0f);
    arrow.lineTo (cx + 4.0f, cy - 2.0f);

    g.setColour (Colours::textDim);
    g.strokePath (arrow, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved,
                                               juce::PathStrokeType::rounded));
}

juce::Font DrummyLookAndFeel::getLabelFont (juce::Label& label)
{
    return { juce::FontOptions ((float) juce::jmin (15, label.getHeight() - 2)) };
}

} // namespace drummy::ui
