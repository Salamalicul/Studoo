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
        float lfoRate = 4.0f, lfoAmount = 0.0f;   // LFO que mueve el filtro
        bool mono = false;                        // una nota a la vez: las notas solapadas se deslizan
        float glide = 0.0f;                       // segundos que tarda en deslizarse (modo mono)
    };

    struct SamplerParams
    {
        float tune = 0.0f;              // semitonos
        float start = 0.0f, end = 1.0f; // parte del sample que suena (0..1)
        bool reverse = false;
        int slices = 1;                 // trocear en N partes (cada nota toca un trozo)
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
        SamplerParams sampler;
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
        float gain = 1.0f;                     // clips de audio: volumen
        double timeRatio = 1.0;                // > 1: el audio avanza más deprisa (sigue un tempo más rápido)
        double pitchRatio = 1.0;               // > 1: más agudo (sin cambiar la velocidad)
        double fadeIn = 0.0, fadeOut = 0.0;    // clips de audio: fundidos (en pasos)
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
        float filter = 0.0f;        // -1 paso bajo .. 0 nada .. 1 paso alto
        float distortion = 0.0f, chorusMix = 0.0f;
        float sidechain = 0.0f;     // 0..1: cuánto se agacha cuando suena la pista de origen
        int sidechainSource = 1;
        int sendTo = 0;             // 0 = sin envío; si no, una pista de número MAYOR (así no hay bucles)
        float sendLevel = 0.0f;
        bool sendOnly = false;      // solo suena por el envío (agrupar pistas en un "bus")
        bool limiter = false;
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
        float swing = 0.0f;          // 0..1: retrasa las semicorcheas pares
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
