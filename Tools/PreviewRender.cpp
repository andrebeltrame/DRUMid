/*  Renders the real plugin editor straight to PNG - no window, no screen.

    cmake -B build -DDRUMID_BUILD_PREVIEW=ON
    cmake --build build --target DrumidPreview && ./build/.../DrumidPreview <outdir>
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace drumid;

static bool renderOne (Genre genre, float energy, float complexity, int bars,
                       const juce::File& outFile, bool withAbout = false)
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

    std::unique_ptr<juce::AudioProcessorEditor> editor (proc.createEditor());

    if (editor == nullptr)
        return false;

    const int w = 980, h = 500;
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

    const Shot shots[] =
    {
        { Genre::OrganicHouse,  0.60f, 0.60f, 2, "drumid-organic-house"  },
        { Genre::AfroHouse,     0.65f, 0.60f, 2, "drumid-afro-house"     },
        { Genre::IndieDance,    0.55f, 0.50f, 2, "drumid-indie-dance"    },
        { Genre::MelodicHouse,  0.55f, 0.40f, 2, "drumid-melodic-house"  },
        { Genre::MelodicTechno, 0.50f, 0.45f, 2, "drumid-melodic-techno" },
        { Genre::Techno,        0.60f, 0.50f, 2, "drumid-techno"         },
    };

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
