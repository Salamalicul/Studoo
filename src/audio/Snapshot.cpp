#include "Snapshot.h"
#include "model/Scales.h"
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
    snap->swing = num (state, ids::swing, 0.0f, 1.0f, 0.0f);
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
        for (auto z : ch)
        {
            if (! z.hasType (ids::ZONE) || c.zones.size() >= 128)
                continue;
            ChannelSnap::Zone zone;
            zone.path = z[ids::sample].toString();
            zone.root = juce::jlimit (0, 127, (int) z.getProperty (ids::root, 60));
            zone.lo = juce::jlimit (0, 127, (int) z.getProperty (ids::lo, 0));
            zone.hi = juce::jlimit (zone.lo, 127, (int) z.getProperty (ids::hi, 127));
            if (zone.path.isNotEmpty())
                c.zones.push_back (zone);
        }

        c.synth.wave = juce::jlimit (0, 5, (int) ch.getProperty (ids::wave, 0));
        c.synth.timbre = num (ch, ids::timbre, 0.0f, 1.0f, 0.5f);
        c.synth.fmRatio = juce::jlimit (1, 8, (int) ch.getProperty (ids::fmRatio, 2));
        c.synth.attack = num (ch, ids::attack, 0.001f, 2.0f, 0.005f);
        c.synth.decay = num (ch, ids::decay, 0.01f, 3.0f, 0.3f);
        c.synth.sustain = num (ch, ids::sustain, 0.0f, 1.0f, 0.6f);
        c.synth.release = num (ch, ids::release, 0.005f, 4.0f, 0.3f);
        c.synth.cutoff = num (ch, ids::cutoff, 20.0f, 20000.0f, 5000.0f);
        c.synth.resonance = num (ch, ids::resonance, 0.0f, 1.0f, 0.2f);
        c.synth.detune = num (ch, ids::detune, 0.0f, 50.0f, 8.0f);
        c.synth.lfoRate = num (ch, ids::lfoRate, 0.1f, 20.0f, 4.0f);
        c.synth.lfoAmount = num (ch, ids::lfoAmount, 0.0f, 1.0f, 0.0f);
        c.synth.mono = (bool) ch.getProperty (ids::mono, false);
        c.synth.glide = num (ch, ids::glide, 0.0f, 1.0f, 0.0f);

        c.sampler.tune = num (ch, ids::sampleTune, -24.0f, 24.0f, 0.0f);
        c.sampler.start = num (ch, ids::sampleStart, 0.0f, 0.99f, 0.0f);
        c.sampler.end = juce::jmax (c.sampler.start + 0.01f, num (ch, ids::sampleEnd, 0.01f, 1.0f, 1.0f));
        c.sampler.reverse = (bool) ch[ids::reverse];
        c.sampler.slices = juce::jlimit (1, 64, (int) ch.getProperty (ids::slices, 1));
        {
            // Golpes detectados: solo valen si son tantos como "Trozos" y van en orden
            const auto points = juce::StringArray::fromTokens (ch[ids::slicePoints].toString(), ",", "");
            if (points.size() == c.sampler.slices && points.size() > 1 && points.size() <= 64)
            {
                float prev = -1.0f;
                bool ok = true;
                for (int i = 0; i < points.size() && ok; ++i)
                {
                    const float v = points[i].getFloatValue();
                    ok = std::isfinite (v) && v >= 0.0f && v < 1.0f && v > prev;
                    c.sampler.slicePoints[(size_t) i] = v;
                    prev = v;
                }
                c.sampler.numSlicePoints = ok ? points.size() : 0;
            }
            if (c.sampler.numSlicePoints == 0)
                c.sampler.slices = juce::jmin (16, c.sampler.slices);
        }

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
                const auto vels = juce::StringArray::fromTokens (item[ids::stepVel].toString(), ",", "");
                const auto pitches = juce::StringArray::fromTokens (item[ids::stepPitch].toString(), ",", "");
                const double stepLength = ch.type == ChannelType::bass808 ? 4.0 : 1.0;
                for (int s = 0; s < juce::jmin (data.length(), (int) ps.length); ++s)
                    if (data[s] == '1')
                    {
                        // Editor de pasos: fuerza y tono de cada golpe (con valores seguros)
                        const float vel = s < vels.size() && vels[s].isNotEmpty() ? juce::jlimit (0, 100, vels[s].getIntValue()) / 100.0f : 0.85f;
                        const int pitch = juce::jlimit (0, 127, ch.root + (s < pitches.size() ? juce::jlimit (-24, 24, pitches[s].getIntValue()) : 0));
                        if (vel > 0.0f)
                            notes.push_back ({ (double) s, stepLength, pitch, vel });
                    }
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

    std::map<int, std::vector<LaneSnap::Block>> automationBlocks;   // uid de la línea -> bloques
    // --- Playlist ---------------------------------------------------------------
    double songEnd = 0.0;
    for (auto c : state.getChildWithName (ids::PLAYLIST))
    {
        ClipSnap cs;
        if (c[ids::kind].toString() == "automation")
        {
            // Bloque de automatización: se guarda para su línea (se usa al leer las automatizaciones)
            const LaneSnap::Block block { pos (c[ids::start], 16000.0), juce::jmax (0.25, pos (c[ids::length], 16000.0)) };
            automationBlocks[(int) c[ids::ref]].push_back (block);
            songEnd = juce::jmax (songEnd, block.start + block.length);
            continue;
        }
        cs.isAudio = c[ids::kind].toString() == "audio";
        if ((bool) c.getProperty (ids::clipMuted, false))
            continue;   // clip en silencio (otra toma elegida)
        // Una canción de Studoo puede durar como mucho 1000 compases
        constexpr double maxSong = 16000.0;
        cs.start = pos (c[ids::start], maxSong);
        cs.length = juce::jmax (0.25, pos (c[ids::length], maxSong));
        cs.offset = pos (c[ids::offset], maxSong);
        cs.gain = num (c, ids::clipGain, 0.0f, 2.0f, 1.0f);
        if ((bool) c.getProperty (ids::stretch, false))
            cs.timeRatio = juce::jlimit (0.25, 4.0, (double) snap->bpm / (double) num (c, ids::recordBpm, 40.0f, 250.0f, snap->bpm));
        cs.pitchRatio = std::pow (2.0, (double) juce::roundToInt (num (c, ids::clipPitch, -12.0f, 12.0f, 0.0f)) / 12.0);
        cs.fadeIn = juce::jmin (cs.length * 0.5, pos (c[ids::fadeIn], maxSong));
        cs.fadeOut = juce::jmin (cs.length * 0.5, pos (c[ids::fadeOut], maxSong));

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
    snap->stepsPerBar = Project::stepsPerBarOf (state);
    snap->songLength = juce::jmax ((double) snap->stepsPerBar, std::ceil (songEnd / snap->stepsPerBar) * snap->stepsPerBar);

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
        m.filter = num (t, ids::trackFilter, -1.0f, 1.0f, 0.0f);
        m.distortion = num (t, ids::distortion, 0.0f, 1.0f, 0.0f);
        m.chorusMix = num (t, ids::chorusMix, 0.0f, 1.0f, 0.0f);
        m.sidechain = num (t, ids::sidechain, 0.0f, 1.0f, 0.0f);
        m.sidechainSource = juce::jlimit (1, numMixerTracks - 1, (int) t.getProperty (ids::sidechainSource, 1));
        // Envíos: solo hacia una pista de número mayor (evita bucles infinitos); el Master no envía
        const int sendTo = t.getProperty (ids::sendTo, 0);
        m.sendTo = (i > 0 && sendTo > i && sendTo < numMixerTracks) ? sendTo : 0;
        m.sendLevel = m.sendTo > 0 ? num (t, ids::sendLevel, 0.0f, 1.0f, 0.0f) : 0.0f;
        m.sendOnly = m.sendTo > 0 && (bool) t.getProperty (ids::sendOnly, false);
        for (int b = 0; b < numEqBands; ++b)
        {
            static const float defaults[] = { 100.0f, 400.0f, 2000.0f, 8000.0f };
            m.peqFreq[(size_t) b] = num (t, peqId ("Freq", b), 20.0f, 20000.0f, defaults[b]);
            m.peqGain[(size_t) b] = num (t, peqId ("Gain", b), -18.0f, 18.0f, 0.0f);
            m.peqQ[(size_t) b] = num (t, peqId ("Q", b), 0.2f, 10.0f, 1.0f);
        }
        m.gateThreshold = num (t, ids::gateThreshold, -80.0f, 0.0f, -80.0f);
        m.tuneAmount = num (t, ids::tuneAmount, 0.0f, 1.0f, 0.0f);
        m.tuneRoot = juce::jlimit (0, 11, (int) t.getProperty (ids::tuneRoot, 0));
        m.tuneScale = juce::jlimit (0, scales::numScales - 1, (int) t.getProperty (ids::tuneScale, 0));
        m.rhythmMode = juce::jlimit (0, 6, (int) t.getProperty (ids::rhythmMode, 0));
        m.rhythmMix = num (t, ids::rhythmMix, 0.0f, 1.0f, 1.0f);
        m.maximize = num (t, ids::maximize, 0.0f, 12.0f, 0.0f);
        for (int s = 1; s < numSends; ++s)
        {
            const int to = t.getProperty (sendToId (s), 0);
            const bool valid = i > 0 && to > i && to < numMixerTracks;
            m.moreSendTo[(size_t) s - 1] = valid ? to : 0;
            m.moreSendLevel[(size_t) s - 1] = valid ? num (t, sendLevelId (s), 0.0f, 1.0f, 0.0f) : 0.0f;
        }
        // El Master trae el limitador encendido (protege de que suene a tope)
        m.limiter = (bool) t.getProperty (ids::limiter, i == 0);
    }

    // --- Automatización -------------------------------------------------------------
    for (auto lane : state.getChildWithName (ids::AUTOMATION))
    {
        LaneSnap ls;
        ls.target = AutomationTarget::parse (lane[ids::target].toString());
        if (! ls.target.isValid())
            continue;

        if (ls.target.kind == AutomationTarget::Kind::project)
        {
            if (snap->tempoLane >= 0)
                continue;   // solo una línea de tempo
        }
        else if (ls.target.kind == AutomationTarget::Kind::channel)
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
        if (auto it = automationBlocks.find ((int) lane[ids::uid]); it != automationBlocks.end())
        {
            ls.blocks = it->second;
            std::sort (ls.blocks.begin(), ls.blocks.end(), [] (auto& a, auto& b) { return a.start < b.start; });
            if (ls.blocks.size() > 2000) ls.blocks.resize (2000);
        }
        std::sort (ls.points.begin(), ls.points.end());
        if (! ls.points.empty())
        {
            if (ls.target.kind == AutomationTarget::Kind::project)
                snap->tempoLane = (int) snap->lanes.size();
            snap->lanes.push_back (std::move (ls));
        }
    }

    return snap;
}
}
