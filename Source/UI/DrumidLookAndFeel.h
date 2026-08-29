#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace drumid::ui
{

namespace Colours
{
    const juce::Colour background   { 0xff14161a };
    const juce::Colour panel        { 0xff1c1f26 };
    const juce::Colour panelLight   { 0xff242832 };
    const juce::Colour outline      { 0xff2e3340 };
    const juce::Colour text         { 0xffd8dde8 };
    const juce::Colour textDim      { 0xff7b8496 };
    const juce::Colour accent       { 0xffffb13d };   // hits
    const juce::Colour accentSoft   { 0xff8a5f24 };   // ghosts
    const juce::Colour playhead     { 0xff3ddc97 };
    const juce::Colour locked       { 0xff5ba8ff };
    const juce::Colour danger       { 0xffe0554b };
}

class DrumidLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DrumidLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&,
                               bool highlighted, bool down) override;

    void drawComboBox (juce::Graphics&, int w, int h, bool down,
                       int bx, int by, int bw, int bh, juce::ComboBox&) override;

    juce::Font getLabelFont (juce::Label&) override;
};

} // namespace drumid::ui
