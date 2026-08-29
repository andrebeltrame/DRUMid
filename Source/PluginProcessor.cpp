#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace drummy;

DrummyAudioProcessor::DrummyAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    resetToDefaults();
}

void DrummyAudioProcessor::resetToDefaults()
{
    editKit.setBars (gen.bars);
    NoteMap::apply (editKit, notePreset);

    generateAll (false);
}

// ============================================================================

void DrummyAudioProcessor::prepareToPlay (double sampleRate, int)
{
    sequencer.prepare (sampleRate);
    publishKit();
}

bool DrummyAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
}

void DrummyAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    // Drummy makes no sound of its own - the Drum Rack does.
    buffer.clear();

    // Incoming MIDI is not passed through; the host track is a generator, not a
    // thru. (Keeping it would double-trigger the rack when you play the keys.)
    midi.clear();

    const auto& k = audioKits[(size_t) activeKit.load()];
    const int n = buffer.getNumSamples();

    bool playing = false;
    double ppq = 0.0;
    double bpm = 120.0;

    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            playing = pos->getIsPlaying();
            ppq = pos->getPpqPosition().orFallback (0.0);
            bpm = pos->getBpm().orFallback (120.0);
        }
    }

    hostPlaying.store (playing);

    if (playing)
        sequencer.process (k, midi, n, ppq, bpm, true);
    else if (freeRun.load())
        sequencer.processFreeRunning (k, midi, n, freeBpm.load());
    else
        sequencer.process (k, midi, n, ppq, bpm, false);
}

// ============================================================================

void DrummyAudioProcessor::publishKit()
{
    const int inactive = 1 - activeKit.load();
    audioKits[(size_t) inactive] = editKit;
    activeKit.store (inactive);
}

void DrummyAudioProcessor::setNoteMapPreset (NoteMapPreset p)
{
    notePreset = p;
    NoteMap::apply (editKit, p);
    publishKit();
    kitChanged.sendChangeMessage();
}

void DrummyAudioProcessor::generateAll (bool newSeed)
{
    if (newSeed)
        gen.seed = juce::Random::getSystemRandom().nextInt ({ 1, 1000000 });

    Generator::generate (editKit, gen);
    publishKit();
    kitChanged.sendChangeMessage();
}

void DrummyAudioProcessor::generateLane (LaneId lane)
{
    const int idx = (int) lane;
    const int laneSeed = juce::Random::getSystemRandom().nextInt ({ 1, 1000000 }) + idx;

    Generator::generateLane (editKit, lane, gen, laneSeed);
    publishKit();
    kitChanged.sendChangeMessage();
}

// ============================================================================
//  state
// ============================================================================

void DrummyAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    juce::ValueTree state ("DRUMMY");
    state.setProperty ("version", 1, nullptr);
    state.setProperty ("genre", (int) gen.genre, nullptr);
    state.setProperty ("energy", gen.energy, nullptr);
    state.setProperty ("complexity", gen.complexity, nullptr);
    state.setProperty ("swing", gen.swing, nullptr);
    state.setProperty ("humanTiming", gen.humanTiming, nullptr);
    state.setProperty ("humanVel", gen.humanVel, nullptr);
    state.setProperty ("bars", gen.bars, nullptr);
    state.setProperty ("seed", gen.seed, nullptr);
    state.setProperty ("fills", gen.fills, nullptr);
    state.setProperty ("noteMap", (int) notePreset, nullptr);
    state.setProperty ("freeRun", freeRun.load(), nullptr);
    state.setProperty ("freeBpm", freeBpm.load(), nullptr);

    for (int i = 0; i < kNumLanes; ++i)
    {
        juce::ValueTree lane ("LANE");
        const auto& ls = editKit.lanes[(size_t) i];

        lane.setProperty ("index", i, nullptr);
        lane.setProperty ("enabled", ls.enabled, nullptr);
        lane.setProperty ("locked", ls.locked, nullptr);
        lane.setProperty ("note", ls.midiNote, nullptr);
        lane.setProperty ("gain", ls.gain, nullptr);

        // Steps are packed as "on:vel:micro:prob:ratchet" per step, comma joined.
        juce::StringArray packed;
        const auto& p = editKit.patterns[(size_t) i];

        for (int s = 0; s < editKit.numSteps; ++s)
        {
            const auto& st = p.steps[(size_t) s];
            packed.add (juce::String ((int) st.on) + ":"
                        + juce::String (st.velocity, 3) + ":"
                        + juce::String (st.micro, 3) + ":"
                        + juce::String (st.probability, 3) + ":"
                        + juce::String (st.ratchet));
        }

        lane.setProperty ("steps", packed.joinIntoString (","), nullptr);
        state.appendChild (lane, nullptr);
    }

    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, dest);
}

void DrummyAudioProcessor::setStateInformation (const void* data, int size)
{
    auto xml = getXmlFromBinary (data, size);

    if (xml == nullptr)
        return;

    auto state = juce::ValueTree::fromXml (*xml);

    if (! state.hasType ("DRUMMY"))
        return;

    gen.genre       = (Genre) (int) state.getProperty ("genre", 0);
    gen.energy      = (float) state.getProperty ("energy", 0.5);
    gen.complexity  = (float) state.getProperty ("complexity", 0.4);
    gen.swing       = (float) state.getProperty ("swing", 0.52);
    gen.humanTiming = (float) state.getProperty ("humanTiming", 0.15);
    gen.humanVel    = (float) state.getProperty ("humanVel", 0.25);
    gen.bars        = (int)   state.getProperty ("bars", 2);
    gen.seed        = (int)   state.getProperty ("seed", 1);
    gen.fills       = (bool)  state.getProperty ("fills", true);
    notePreset      = (NoteMapPreset) (int) state.getProperty ("noteMap", 0);
    freeRun.store ((bool) state.getProperty ("freeRun", false));
    freeBpm.store ((double) state.getProperty ("freeBpm", 122.0));

    editKit.setBars (gen.bars);

    for (auto lane : state)
    {
        const int i = (int) lane.getProperty ("index", -1);

        if (i < 0 || i >= kNumLanes)
            continue;

        auto& ls = editKit.lanes[(size_t) i];
        ls.enabled  = (bool) lane.getProperty ("enabled", true);
        ls.locked   = (bool) lane.getProperty ("locked", false);
        ls.midiNote = (int)  lane.getProperty ("note", 36);
        ls.gain     = (float) lane.getProperty ("gain", 1.0);

        auto& p = editKit.patterns[(size_t) i];
        p.clear();
        p.numSteps = editKit.numSteps;

        juce::StringArray packed;
        packed.addTokens (lane.getProperty ("steps", "").toString(), ",", "");

        for (int s = 0; s < packed.size() && s < p.numSteps; ++s)
        {
            juce::StringArray f;
            f.addTokens (packed[s], ":", "");

            if (f.size() < 5)
                continue;

            auto& st = p.steps[(size_t) s];
            st.on          = f[0].getIntValue() != 0;
            st.velocity    = f[1].getFloatValue();
            st.micro       = f[2].getFloatValue();
            st.probability = f[3].getFloatValue();
            st.ratchet     = juce::jlimit (1, 4, f[4].getIntValue());
        }
    }

    publishKit();
    kitChanged.sendChangeMessage();
}

// ============================================================================

juce::AudioProcessorEditor* DrummyAudioProcessor::createEditor()
{
    return new DrummyAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DrummyAudioProcessor();
}
