#include "MidiIO.h"

namespace studoo::midiio
{
namespace
{
    constexpr int ticksPerQuarter = 96;
    constexpr int ticksPerStep = ticksPerQuarter / stepsPerBeat;   // 24
}

juce::MidiFile patternToMidi (Project& project, juce::ValueTree pattern)
{
    juce::MidiFile file;
    file.setTicksPerQuarterNote (ticksPerQuarter);

    // Pista 0: el tempo
    {
        juce::MidiMessageSequence tempo;
        const double bpm = juce::jlimit (40.0, 250.0, (double) project.getState().getProperty (ids::bpm, 140.0));
        tempo.addEvent (juce::MidiMessage::tempoMetaEvent (juce::roundToInt (60000000.0 / bpm)), 0.0);
        tempo.addEvent (juce::MidiMessage::timeSignatureMetaEvent (4, 4), 0.0);
        file.addTrack (tempo);
    }

    const int length = Project::getPatternLength (pattern);
    int midiChannel = 1;
    for (auto ch : project.channels())
    {
        const int uid = ch[ids::uid];
        juce::MidiMessageSequence seq;
        seq.addEvent (juce::MidiMessage::textMetaEvent (3, ch[ids::name].toString()), 0.0);
        const int root = juce::jlimit (0, 127, (int) ch.getProperty (ids::root, 60));
        // La batería va al canal 10 (el de percusión en MIDI); el resto, uno por instrumento
        const bool drums = ch[ids::type].toString() == "sampler" && (bool) ch.getProperty (ids::oneShot, true);
        const int mc = drums ? 10 : midiChannel;
        bool any = false;

        for (auto item : pattern)
        {
            if ((int) item[ids::channel] != uid)
                continue;
            if (item.hasType (ids::STEPSET))
            {
                const auto data = item[ids::data].toString();
                for (int s = 0; s < juce::jmin (length, data.length()); ++s)
                    if (data[s] == '1')
                    {
                        seq.addEvent (juce::MidiMessage::noteOn (mc, root, (juce::uint8) 100), s * ticksPerStep);
                        seq.addEvent (juce::MidiMessage::noteOff (mc, root), (s + 1) * ticksPerStep - 1);
                        any = true;
                    }
            }
            else if (item.hasType (ids::NOTE))
            {
                const double start = item[ids::start], len = item[ids::length];
                const int pitch = juce::jlimit (0, 127, (int) item[ids::pitch]);
                const float vel = juce::jlimit (0.01f, 1.0f, (float) item.getProperty (ids::velocity, 0.8f));
                seq.addEvent (juce::MidiMessage::noteOn (mc, pitch, vel), start * ticksPerStep);
                seq.addEvent (juce::MidiMessage::noteOff (mc, pitch), juce::jmax (start * ticksPerStep + 1, (start + len) * ticksPerStep - 1));
                any = true;
            }
        }
        if (any)
        {
            seq.sort();
            seq.updateMatchedPairs();
            file.addTrack (seq);
            if (! drums)
                midiChannel = midiChannel % 16 + 1 == 10 ? 11 : midiChannel % 16 + 1;
        }
    }
    return file;
}

int importIntoPattern (Project& project, juce::ValueTree pattern, int channelUid, const juce::MidiFile& source)
{
    const short format = source.getTimeFormat();
    if (format <= 0 || ! project.findChannel (channelUid).isValid())
        return 0;   // los archivos con tiempo SMPTE (muy raros) no se admiten
    const double stepsPerTick = (double) stepsPerBeat / (double) format;

    struct Imported { double start, length; int pitch; float velocity; };
    std::vector<Imported> notes;
    double end = 0.0;
    for (int t = 0; t < source.getNumTracks(); ++t)
    {
        juce::MidiMessageSequence seq (*source.getTrack (t));
        seq.updateMatchedPairs();
        for (int i = 0; i < seq.getNumEvents() && notes.size() < 4000; ++i)
        {
            auto* e = seq.getEventPointer (i);
            if (! e->message.isNoteOn())
                continue;
            const double start = e->message.getTimeStamp() * stepsPerTick;
            const double off = e->noteOffObject != nullptr ? e->noteOffObject->message.getTimeStamp() * stepsPerTick : start + 1.0;
            if (! std::isfinite (start) || start < 0.0 || start >= 256.0)
                continue;
            const double len = juce::jlimit (0.25, 256.0 - start, std::round ((off - start) * 4.0) / 4.0);
            notes.push_back ({ std::round (start * 4.0) / 4.0, len, e->message.getNoteNumber(), e->message.getFloatVelocity() });
            end = juce::jmax (end, start + len);
        }
    }
    if (notes.empty())
        return 0;

    auto* um = project.um();
    const int needed = end > 128.0 ? 256 : end > 64.0 ? 128 : end > 32.0 ? 64 : end > 16.0 ? 32 : 16;
    if (Project::getPatternLength (pattern) < needed)
        pattern.setProperty (ids::length, needed, um);
    for (auto& n : notes)
        project.addNote (pattern, channelUid, n.start, n.length, juce::jlimit (0, 127, n.pitch), juce::jlimit (0.05f, 1.0f, n.velocity));
    return (int) notes.size();
}
}
