#pragma once

#include "DrumidLookAndFeel.h"

namespace drumid::ui
{

/** The about card, shown over the editor.

    Covers the whole editor and dims what is behind it, so the version is
    somewhere you can always find it without it taking up room in the header.
*/
class AboutPanel : public juce::Component
{
public:
    AboutPanel();

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

private:
    juce::Rectangle<float> cardArea() const;

    juce::TextButton closeButton { "CLOSE" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AboutPanel)
};

} // namespace drumid::ui
