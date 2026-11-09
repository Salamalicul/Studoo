#include "Snapshot.h"
#include <algorithm>
#include <map>

namespace studoo
{
std::shared_ptr<Snapshot> Snapshot::build (const juce::ValueTree& state)
{
    auto snap = std::make_shared<Snapshot>();

    snap->bpm = juce::jlimit (40.0, 250.0, (double) state.getProperty (ids::bpm, 140.0));
    snap->metronome = (bool) state[ids::metronome];
    snap->songMode = state[ids::playMode].toString() == "song";

    // --- Canales --------------------------------------------------------------
    std::map<int, int> channelIndexByUid;
    for (auto ch : state.getChildWithName (ids::CHANNELS))
    {
        ChannelSnap c;
        c.uid = ch[ids::uid];
        c.type = channelTypeFromString (ch[ids::type].toString());
        c.volume = ch.getProperty (ids::volume, 0.8f);
        c.pan = ch.getProperty (ids::pan, 0.0f);
        c.muted = ch[ids::muted];
        c.mixer = juce::jlimit (0, numMixerTracks - 1, (int) ch[ids::mixer]);
        c.root = ch.getProperty (ids::root, 60);
        c.oneShot = ch.getProperty (ids::oneShot, true);
        c.samplePath = ch[ids::sample].toString();

        c.synth.wave = ch.getProperty (ids::wave, 0);
        c.synth.attack = ch.getProperty (ids::attack, 0.005f);
        c.synth.decay = ch.getProperty (ids::decay, 0.3f);
        c.synth.sustain = ch.getProperty (ids::sustain, 0.6f);
        c.synth.release = ch.getProperty (ids::release, 0.3f);
        c.synth.cutoff = ch.getProperty (ids::cutoff, 5000.0f);
        c.synth.resonance = ch.getProperty (ids::resonance, 0.2f);
        c.synth.detune = ch.getProperty (ids::detune, 8.0f);

        c.bass.decay = ch.getProperty (ids::decay, 1.5f);
        c.bass.glide = ch.getProperty (ids::glide, 0.08f);
        c.bass.drive = ch.getProperty (ids::drive, 0.3f);

        channelIndexByUid[c.uid] = (int) snap->channels.size();
        snap->channels.push_back (c);
    }

    const auto numChannels = snap->channels.size();

    // --- Patrones -------------------------------------------------------------
    std::map<int, int> patternIndexByUid;
    const int currentUid = state[ids::currentPattern];

    for (auto p : state.getChildWithName (ids::PATTERNS))
    {
        PatternSnap ps;
        ps.uid = p[ids::uid];
        ps.length = Project::getPatternLength (p);
        ps.notesPerChannel.resize (numChannels);

        for (auto item : p)
        {
            auto it = channelIndexByUid.find ((int) item[ids::channel]);
            if (it == channelIndexByUid.end())
                continue;
            const auto& ch = snap->channels[(size_t) it->second];
            auto& notes = ps.notesPerChannel[(size_t) it->second];

            if (item.hasType (ids::STEPSET))
            {
                if (ch.type == ChannelType::audio)
                    continue;
                // Cada paso encendido se convierte en una nota
                const auto data = item[ids::data].toString();
                const double stepLength = ch.type == ChannelType::bass808 ? 4.0 : 1.0;
                for (int s = 0; s < juce::jmin (data.length(), (int) ps.length); ++s)
                    if (data[s] == '1')
                        notes.push_back ({ (double) s, stepLength, ch.root, 0.85f });
            }
            else if (item.hasType (ids::NOTE))
            {
                NoteEvent n;
                n.start = item[ids::start];
                n.length = juce::jmax (0.05, (double) item[ids::length]);
                n.pitch = juce::jlimit (0, 127, (int) item[ids::pitch]);
                n.velocity = juce::jlimit (0.0f, 1.0f, (float) item.getProperty (ids::velocity, 0.8f));
                if (n.start < ps.length)
                    notes.push_back (n);
            }
        }

        for (auto& notes : ps.notesPerChannel)
            std::sort (notes.begin(), notes.end(), [] (const NoteEvent& a, const NoteEvent& b) { return a.start < b.start; });

        if (ps.uid == currentUid)
            snap->currentPatternIndex = (int) snap->patterns.size();

        patternIndexByUid[ps.uid] = (int) snap->patterns.size();
        snap->patterns.push_back (std::move (ps));
    }

    // --- Playlist ---------------------------------------------------------------
    double songEnd = 0.0;
    for (auto c : state.getChildWithName (ids::PLAYLIST))
    {
        ClipSnap cs;
        cs.isAudio = c[ids::kind].toString() == "audio";
        cs.start = c[ids::start];
        cs.length = juce::jmax (0.25, (double) c[ids::length]);
        cs.offset = c[ids::offset];

        if (cs.isAudio)
        {
            auto it = channelIndexByUid.find ((int) c[ids::ref]);
            if (it == channelIndexByUid.end()) continue;
            cs.channelIndex = it->second;
        }
        else
        {
            auto it = patternIndexByUid.find ((int) c[ids::ref]);
            if (it == patternIndexByUid.end()) continue;
            cs.patternIndex = it->second;
        }

        songEnd = juce::jmax (songEnd, cs.start + cs.length);
        snap->clips.push_back (cs);
    }
    snap->songLength = juce::jmax ((double) stepsPerBar, std::ceil (songEnd / stepsPerBar) * stepsPerBar);

    // --- Mezclador ----------------------------------------------------------------
    for (auto t : state.getChildWithName (ids::MIXER))
    {
        const int i = t[ids::index];
        if (! juce::isPositiveAndBelow (i, numMixerTracks))
            continue;
        auto& m = snap->mixer[(size_t) i];
        m.volume = t.getProperty (ids::volume, 0.8f);
        m.pan = t.getProperty (ids::pan, 0.0f);
        m.mute = t[ids::muted];
        m.solo = t[ids::solo];
        m.eqLow = t.getProperty (ids::eqLow, 0.0f);
        m.eqMid = t.getProperty (ids::eqMid, 0.0f);
        m.eqHigh = t.getProperty (ids::eqHigh, 0.0f);
        m.compOn = t[ids::compOn];
        m.compThreshold = t.getProperty (ids::compThreshold, -18.0f);
        m.compRatio = t.getProperty (ids::compRatio, 4.0f);
        m.delayMix = t.getProperty (ids::delayMix, 0.0f);
        m.delayTime = t.getProperty (ids::delayTime, 0.75f);
        m.delayFeedback = t.getProperty (ids::delayFeedback, 0.35f);
        m.reverbMix = t.getProperty (ids::reverbMix, 0.0f);
        m.reverbSize = t.getProperty (ids::reverbSize, 0.6f);
    }

    // --- Automatización -------------------------------------------------------------
    for (auto lane : state.getChildWithName (ids::AUTOMATION))
    {
        LaneSnap ls;
        ls.target = AutomationTarget::parse (lane[ids::target].toString());
        if (! ls.target.isValid())
            continue;

        if (ls.target.kind == AutomationTarget::Kind::channel)
        {
            auto it = channelIndexByUid.find (ls.target.index);
            if (it == channelIndexByUid.end()) continue;
            ls.channelIndex = it->second;
        }
        else if (! juce::isPositiveAndBelow (ls.target.index, numMixerTracks))
        {
            continue;
        }

        for (auto p : lane)
            ls.points.emplace_back ((double) p[ids::time], (float) p[ids::value]);
        std::sort (ls.points.begin(), ls.points.end());
        if (! ls.points.empty())
            snap->lanes.push_back (std::move (ls));
    }

    return snap;
}
}
