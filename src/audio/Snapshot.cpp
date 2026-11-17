#include "Snapshot.h"
#include <algorithm>
#include <cmath>
#include <map>

namespace studoo
{
namespace
{
    // Seguridad: un proyecto puede venir de otra persona (o estar estropeado).
    // Todos los números se limitan a los mismos rangos que los controles de la
    // app, y los valores raros (infinito, "no es un número") se cambian por el
    // valor normal. Así un archivo no puede romper el motor de audio ni hacer
    // que suene a un volumen peligroso.
    float num (const juce::ValueTree& t, const juce::Identifier& id, float lo, float hi, float def)
    {
        const auto v = (double) t.getProperty (id, def);
        return std::isfinite (v) ? juce::jlimit (lo, hi, (float) v) : def;
    }

    double pos (const juce::var& value, double hi = 1.0e6)
    {
        const auto v = (double) value;
        return std::isfinite (v) ? juce::jlimit (0.0, hi, v) : 0.0;
    }
}

std::shared_ptr<Snapshot> Snapshot::build (const juce::ValueTree& state)
{
    auto snap = std::make_shared<Snapshot>();

    snap->bpm = num (state, ids::bpm, 40.0f, 250.0f, 140.0f);
    snap->metronome = (bool) state[ids::metronome];
    snap->songMode = state[ids::playMode].toString() == "song";

    // --- Canales --------------------------------------------------------------
    std::map<int, int> channelIndexByUid;
    for (auto ch : state.getChildWithName (ids::CHANNELS))
    {
        ChannelSnap c;
        c.uid = ch[ids::uid];
        c.type = channelTypeFromString (ch[ids::type].toString());
        c.volume = num (ch, ids::volume, 0.0f, 1.25f, 0.8f);
        c.pan = num (ch, ids::pan, -1.0f, 1.0f, 0.0f);
        c.muted = ch[ids::muted];
        c.mixer = juce::jlimit (0, numMixerTracks - 1, (int) ch[ids::mixer]);
        c.root = juce::jlimit (0, 127, (int) ch.getProperty (ids::root, 60));
        c.oneShot = ch.getProperty (ids::oneShot, true);
        c.samplePath = ch[ids::sample].toString();

        c.synth.wave = juce::jlimit (0, 3, (int) ch.getProperty (ids::wave, 0));
        c.synth.attack = num (ch, ids::attack, 0.001f, 2.0f, 0.005f);
        c.synth.decay = num (ch, ids::decay, 0.01f, 3.0f, 0.3f);
        c.synth.sustain = num (ch, ids::sustain, 0.0f, 1.0f, 0.6f);
        c.synth.release = num (ch, ids::release, 0.005f, 4.0f, 0.3f);
        c.synth.cutoff = num (ch, ids::cutoff, 20.0f, 20000.0f, 5000.0f);
        c.synth.resonance = num (ch, ids::resonance, 0.0f, 1.0f, 0.2f);
        c.synth.detune = num (ch, ids::detune, 0.0f, 50.0f, 8.0f);

        c.bass.decay = num (ch, ids::decay, 0.1f, 4.0f, 1.5f);
        c.bass.glide = num (ch, ids::glide, 0.0f, 0.5f, 0.08f);
        c.bass.drive = num (ch, ids::drive, 0.0f, 1.0f, 0.3f);

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
                n.start = pos (item[ids::start]);
                n.length = juce::jmax (0.05, pos (item[ids::length], 4096.0));
                n.pitch = juce::jlimit (0, 127, (int) item[ids::pitch]);
                n.velocity = num (item, ids::velocity, 0.0f, 1.0f, 0.8f);
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
        // Una canción de Studoo puede durar como mucho 1000 compases
        constexpr double maxSong = 16000.0;
        cs.start = pos (c[ids::start], maxSong);
        cs.length = juce::jmax (0.25, pos (c[ids::length], maxSong));
        cs.offset = pos (c[ids::offset], maxSong);

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
        m.volume = num (t, ids::volume, 0.0f, 1.25f, 0.8f);
        m.pan = num (t, ids::pan, -1.0f, 1.0f, 0.0f);
        m.mute = t[ids::muted];
        m.solo = t[ids::solo];
        m.eqLow = num (t, ids::eqLow, -12.0f, 12.0f, 0.0f);
        m.eqMid = num (t, ids::eqMid, -12.0f, 12.0f, 0.0f);
        m.eqHigh = num (t, ids::eqHigh, -12.0f, 12.0f, 0.0f);
        m.compOn = t[ids::compOn];
        m.compThreshold = num (t, ids::compThreshold, -40.0f, 0.0f, -18.0f);
        m.compRatio = num (t, ids::compRatio, 1.0f, 20.0f, 4.0f);
        m.delayMix = num (t, ids::delayMix, 0.0f, 1.0f, 0.0f);
        m.delayTime = num (t, ids::delayTime, 0.25f, 2.0f, 0.75f);
        m.delayFeedback = num (t, ids::delayFeedback, 0.0f, 0.9f, 0.35f);
        m.reverbMix = num (t, ids::reverbMix, 0.0f, 1.0f, 0.0f);
        m.reverbSize = num (t, ids::reverbSize, 0.0f, 1.0f, 0.6f);
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
            ls.points.emplace_back (pos (p[ids::time]), num (p, ids::value, 0.0f, 1.0f, 0.5f));
        std::sort (ls.points.begin(), ls.points.end());
        if (! ls.points.empty())
            snap->lanes.push_back (std::move (ls));
    }

    return snap;
}
}
