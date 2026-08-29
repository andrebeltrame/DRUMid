#pragma once

#include "Engine/Types.h"
#include "Engine/Generator.h"
#include "Engine/NoteMap.h"
#include "Engine/Sequencer.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <atomic>

/** DRUMid - MIDI drum pattern generator for melodic house, organic house and techno.

    Built as an instrument that outputs MIDI (and silent audio) so Ableton can
    route it: put DRUMid on its own MIDI track, then on the Drum Rack track set
    MIDI From -> <DRUMid track> -> DRUMid, Monitor: In. For committing a pattern
    to a clip, drag the MIDI out of the editor instead.
*/
class DrumidAudioProcessor : public juce::AudioProcessor
{
public:
    DrumidAudioProcessor();
    ~DrumidAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Default"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // ---- editor-facing API (message thread only) --------------------------

    drumid::Kit&         kit()      noexcept { return editKit; }
    drumid::GenSettings& settings() noexcept { return gen; }

    drumid::NoteMapPreset noteMapPreset() const noexcept { return notePreset; }
    void setNoteMapPreset (drumid::NoteMapPreset p);

    /** One-click generate: every enabled, unlocked lane gets a fresh pattern. */
    void generateAll (bool newSeed = true);

    /** Surprise: rolls the genre and every generator setting, then regenerates.
        Locked lanes and the bar count survive - lock is a promise, and pattern
        length is a structural decision, not a flavour. */
    void randomizeAll();
    void generateLane (drumid::LaneId lane);

    /** Copy the edited kit over to the audio thread. Call after any edit. */
    void publishKit();

    int  currentStep() const noexcept { return sequencer.currentStep(); }
    bool isHostPlaying() const noexcept { return hostPlaying.load(); }

    juce::ChangeBroadcaster kitChanged;

private:
    void resetToDefaults();

    drumid::Kit         editKit;
    drumid::GenSettings gen;
    drumid::NoteMapPreset notePreset = drumid::NoteMapPreset::SingleNoteC3;

    std::array<drumid::Kit, 2> audioKits;
    std::atomic<int> activeKit { 0 };

    drumid::Sequencer sequencer;
    std::atomic<bool> hostPlaying { false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrumidAudioProcessor)
};
