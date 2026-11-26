#include "MidiIO.h"
#include <map>

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

juce::ValueTree patternFromPlayed (Project& project, int channelUid, const std::vector<PlayedEvent>& events, double bpm,
                                   double gapSeconds, double maxSeconds)
{
    // Empareja cada nota encendida con su apagado
    struct Played { double start, end; int note; float velocity; };
    std::vector<Played> notes;
    std::map<int, std::pair<double, float>> open;
    for (auto& e : events)
    {
        if (! juce::isPositiveAndBelow (e.note, 128) || ! std::isfinite (e.seconds))
            continue;
        if (e.on && e.velocity > 0.0f)
        {
            if (auto it = open.find (e.note); it != open.end())
                notes.push_back ({ it->second.first, e.seconds, e.note, it->second.second });
            open[e.note] = { e.seconds, e.velocity };
        }
        else if (auto it = open.find (e.note); it != open.end())
        {
            notes.push_back ({ it->second.first, e.seconds, e.note, it->second.second });
            open.erase (it);
        }
    }
    for (auto& [note, o] : open)   // las que siguen pulsadas: hasta el último evento
        notes.push_back ({ o.first, events.empty() ? o.first + 0.25 : juce::jmax (o.first + 0.1, events.back().seconds), note, o.second });
    if (notes.empty())
        return {};
    std::sort (notes.begin(), notes.end(), [] (const Played& a, const Played& b) { return a.start < b.start; });

    // La última frase: hacia atrás hasta un silencio largo
    size_t first = notes.size() - 1;
    double phraseEnd = notes.back().end;
    for (auto& n : notes) phraseEnd = juce::jmax (phraseEnd, n.end);
    while (first > 0)
    {
        double prevEnd = 0.0;
        for (size_t k = 0; k < first; ++k) prevEnd = juce::jmax (prevEnd, notes[k].end);
        if (notes[first].start - prevEnd > gapSeconds || phraseEnd - notes[first - 1].start > maxSeconds)
            break;
        --first;
    }

    const double stepsPerSecond = juce::jlimit (40.0, 250.0, bpm) / 60.0 * stepsPerBeat;
    // Empieza en el compás de la primera nota (redondeando hacia abajo a un tiempo)
    const double t0 = notes[first].start;
    const int bar = project.getStepsPerBar();
    double lastStep = 0.0;
    for (size_t k = first; k < notes.size(); ++k)
        lastStep = juce::jmax (lastStep, (notes[k].end - t0) * stepsPerSecond);
    const int length = juce::jlimit (bar, 256, (int) std::ceil (lastStep / bar - 1.0e-6) * bar);

    auto pattern = project.addPattern (juce::String::fromUTF8 ("Lo que tocaste"));
    pattern.setProperty (ids::length, length, project.um());
    for (size_t k = first; k < notes.size(); ++k)
    {
        const double start = (notes[k].start - t0) * stepsPerSecond;
        if (start >= length)
            break;
        const double len = juce::jlimit (0.25, (double) length - start, (notes[k].end - notes[k].start) * stepsPerSecond);
        project.addNote (pattern, channelUid, start, len, notes[k].note, juce::jlimit (0.05f, 1.0f, notes[k].velocity));
    }
    return pattern;
}
}
