#pragma once

#include "Engine/Types.h"
#include "Engine/Generator.h"
#include "Engine/NoteMap.h"
#include "Engine/Sequencer.h"

#include <juce_audio_processors/juce_audio_processors.h>

#include <array>
#include <atomic>

/** Drummy - MIDI drum pattern generator for melodic house, organic house and techno.

    Built as an instrument that outputs MIDI (and silent audio) so Ableton can
    route it: put Drummy on its own MIDI track, then on the Drum Rack track set
    MIDI From -> <Drummy track> -> Drummy, Monitor: In. For committing a pattern
    to a clip, drag the MIDI out of the editor instead.
*/
class DrummyAudioProcessor : public juce::AudioProcessor
{
public:
    DrummyAudioProcessor();
    ~DrummyAudioProcessor() override = default;

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

    drummy::Kit&         kit()      noexcept { return editKit; }
    drummy::GenSettings& settings() noexcept { return gen; }

    drummy::NoteMapPreset noteMapPreset() const noexcept { return notePreset; }
    void setNoteMapPreset (drummy::NoteMapPreset p);

    /** One-click generate: every enabled, unlocked lane gets a fresh pattern. */
    void generateAll (bool newSeed = true);
    void generateLane (drummy::LaneId lane);

    /** Copy the edited kit over to the audio thread. Call after any edit. */
    void publishKit();

    int  currentStep() const noexcept { return sequencer.currentStep(); }
    bool isHostPlaying() const noexcept { return hostPlaying.load(); }

    /** Standalone has no host transport, so it free-runs on its own clock. */
    bool freeRunEnabled() const noexcept { return freeRun.load(); }
    void setFreeRunEnabled (bool b) noexcept { freeRun.store (b); }
    double freeRunBpm() const noexcept { return freeBpm.load(); }
    void setFreeRunBpm (double b) noexcept { freeBpm.store (juce::jlimit (40.0, 220.0, b)); }

    juce::ChangeBroadcaster kitChanged;

private:
    void resetToDefaults();

    drummy::Kit         editKit;
    drummy::GenSettings gen;
    drummy::NoteMapPreset notePreset = drummy::NoteMapPreset::GeneralMidi;

    std::array<drummy::Kit, 2> audioKits;
    std::atomic<int> activeKit { 0 };

    drummy::Sequencer sequencer;
    std::atomic<bool> hostPlaying { false };
    std::atomic<bool> freeRun { false };
    std::atomic<double> freeBpm { 122.0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrummyAudioProcessor)
};
