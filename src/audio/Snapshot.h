#pragma once
// ============================================================================
//  Snapshot: una "foto" del proyecto preparada para el hilo de audio.
//
//  El hilo de audio no puede leer directamente el árbol del proyecto (podría
//  estar cambiando mientras tanto). Cada vez que editas algo, se crea una
//  foto nueva con todo listo para sonar y se le entrega al motor.
// ============================================================================
#include <juce_core/juce_core.h>
#include <array>
#include <vector>
#include "model/Project.h"

namespace studoo
{
    struct NoteEvent
    {
        double start = 0.0;  // en pasos (semicorcheas)
        double length = 1.0;
        int pitch = 60;
        float velocity = 0.8f;
    };

    struct SynthParams
    {
        int wave = 0; // 0 sierra, 1 cuadrada, 2 seno, 3 triángulo
        float attack = 0.005f, decay = 0.3f, sustain = 0.6f, release = 0.3f;
        float cutoff = 5000.0f, resonance = 0.2f, detune = 8.0f;
    };

    struct Bass808Params
    {
        float decay = 1.5f, glide = 0.08f, drive = 0.3f;
    };

    struct ChannelSnap
    {
        int uid = 0;
        ChannelType type = ChannelType::sampler;
        float volume = 0.8f, pan = 0.0f;
        bool muted = false;
        int mixer = 0;
        int root = 60;
        bool oneShot = true;
        juce::String samplePath;
        SynthParams synth;
        Bass808Params bass;
    };

    struct PatternSnap
    {
        int uid = 0;
        double length = 16.0;
        std::vector<std::vector<NoteEvent>> notesPerChannel; // índice = posición del canal
    };

    struct ClipSnap
    {
        bool isAudio = false;
        int patternIndex = -1; // si es un clip de patrón
        int channelIndex = -1; // si es un clip de audio
        double start = 0.0, length = 16.0, offset = 0.0;
    };

    struct TrackParams
    {
        float volume = 0.8f, pan = 0.0f;
        bool mute = false, solo = false;
        float eqLow = 0.0f, eqMid = 0.0f, eqHigh = 0.0f;
        bool compOn = false;
        float compThreshold = -18.0f, compRatio = 4.0f;
        float delayMix = 0.0f, delayTime = 0.75f, delayFeedback = 0.35f;
        float reverbMix = 0.0f, reverbSize = 0.6f;
    };

    struct LaneSnap
    {
        AutomationTarget target;
        int channelIndex = -1; // si el destino es un canal
        std::vector<std::pair<double, float>> points; // (tiempo en pasos, valor 0..1), ordenados

        float valueAt (double time) const
        {
            if (points.empty()) return 0.0f;
            if (time <= points.front().first) return points.front().second;
            if (time >= points.back().first)  return points.back().second;
            for (size_t i = 1; i < points.size(); ++i)
                if (time < points[i].first)
                {
                    const auto& a = points[i - 1];
                    const auto& b = points[i];
                    const double span = b.first - a.first;
                    const double t = span > 0.0 ? (time - a.first) / span : 0.0;
                    return (float) (a.second + (b.second - a.second) * t);
                }
            return points.back().second;
        }
    };

    struct Snapshot
    {
        double bpm = 140.0;
        bool metronome = false;
        bool songMode = false;
        int currentPatternIndex = 0;
        double songLength = 16.0;

        std::vector<ChannelSnap> channels;
        std::vector<PatternSnap> patterns;
        std::vector<ClipSnap> clips;
        std::array<TrackParams, numMixerTracks> mixer;
        std::vector<LaneSnap> lanes;

        static std::shared_ptr<Snapshot> build (const juce::ValueTree& projectState);
    };
}
