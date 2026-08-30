/*  Renders the real plugin editor straight to PNG - no window, no screen.

    cmake -B build -DDRUMID_BUILD_PREVIEW=ON
    cmake --build build --target DrumidPreview && ./build/.../DrumidPreview <outdir>
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace drumid;

static bool renderOne (Genre genre, float energy, float complexity, int bars,
                       const juce::File& outFile, bool withAbout = false,
                       bool lockDemo = false)
{
    DrumidAudioProcessor proc;

    proc.settings().genre      = genre;
    proc.settings().energy     = energy;
    proc.settings().complexity = complexity;
    proc.settings().bars       = bars;
    proc.settings().swing      = (genre == Genre::Techno)       ? 0.50f
                               : (genre == Genre::OrganicHouse) ? 0.56f
                                                                : 0.53f;
    proc.generateAll (true);

    if (lockDemo)
    {
        // Locked lanes and a disabled one, so the shot shows every state a lane
        // header can be in - closed padlock, open padlock, dimmed.
        proc.kit().lanes[(size_t) LaneId::Kick].locked = true;
        proc.kit().lanes[(size_t) LaneId::Clap].locked = true;
        proc.kit().lanes[(size_t) LaneId::OpenHat].enabled = false;
        proc.publishKit();
    }

    std::unique_ptr<juce::AudioProcessorEditor> editor (proc.createEditor());

    if (editor == nullptr)
        return false;

    const int w = 1020, h = 552;
    editor->setSize (w, h);
    editor->setVisible (true);

    if (withAbout)
        if (auto* de = dynamic_cast<DrumidAudioProcessorEditor*> (editor.get()))
            de->showAbout (true);

    // Render at 2x so the screenshot is readable on a retina display.
    const float scale = 2.0f;
    juce::Image img (juce::Image::ARGB, (int) (w * scale), (int) (h * scale), true);

    {
        juce::Graphics g (img);
        g.addTransform (juce::AffineTransform::scale (scale));
        editor->paintEntireComponent (g, true);
    }

    outFile.deleteFile();

    if (auto stream = outFile.createOutputStream())
    {
        juce::PNGImageFormat png;
        const bool ok = png.writeImageToStream (img, *stream);
        stream->flush();
        return ok;
    }

    return false;
}

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI gui;

    auto outDir = argc > 1 ? juce::File (juce::String (argv[1]))
                           : juce::File::getCurrentWorkingDirectory();
    outDir.createDirectory();

    struct Shot { Genre genre; float energy; float complexity; int bars; const char* name; };

    // One per genre, in the order the selector lists them.
    const Shot shots[] =
    {
        { Genre::Cinematic,        0.55f, 0.45f, 2, "drumid-cinematic"         },
        { Genre::OrganicHouse,     0.60f, 0.60f, 2, "drumid-organic-house"     },
        { Genre::AfroHouse,        0.65f, 0.60f, 2, "drumid-afro-house"        },
        { Genre::IndieDance,       0.55f, 0.50f, 2, "drumid-indie-dance"       },
        { Genre::MelodicHouse,     0.55f, 0.40f, 2, "drumid-melodic-house"     },
        { Genre::ProgressiveHouse, 0.60f, 0.45f, 2, "drumid-progressive-house" },
        { Genre::MelodicTechno,    0.50f, 0.45f, 2, "drumid-melodic-techno"    },
        { Genre::BigRoomEDM,       0.60f, 0.45f, 2, "drumid-big-room-edm"      },
        { Genre::Techno,           0.60f, 0.50f, 2, "drumid-techno"            },
    };

    // Lane header states, for checking the padlock and for the manual.
    {
        auto file = outDir.getChildFile ("drumid-lane-states.png");
        const bool ok = renderOne (Genre::AfroHouse, 0.6f, 0.5f, 2, file, false, true);
        std::printf ("%s  %s\n", ok ? "ok  " : "FAIL", file.getFullPathName().toRawUTF8());
    }

    // The about card, over a kit, so the release screenshot shows the version.
    {
        auto file = outDir.getChildFile ("drumid-about.png");
        const bool ok = renderOne (Genre::MelodicHouse, 0.55f, 0.4f, 2, file, true);
        std::printf ("%s  %s\n", ok ? "ok  " : "FAIL", file.getFullPathName().toRawUTF8());
    }

    int failures = 0;

    for (auto& s : shots)
    {
        auto file = outDir.getChildFile (juce::String (s.name) + ".png");
        const bool ok = renderOne (s.genre, s.energy, s.complexity, s.bars, file);

        std::printf ("%s  %s  (%lld bytes)\n", ok ? "ok  " : "FAIL",
                     file.getFullPathName().toRawUTF8(), (long long) file.getSize());

        if (! ok)
            ++failures;
    }

    return failures == 0 ? 0 : 1;
}
